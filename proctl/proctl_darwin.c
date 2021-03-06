#include "proctl_darwin.h"

static const unsigned char info_plist[]
__attribute__ ((section ("__TEXT,__info_plist"),used)) =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\""
" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
"<plist version=\"1.0\">\n"
"<dict>\n"
"  <key>CFBundleIdentifier</key>\n"
"  <string>org.dlv</string>\n"
"  <key>CFBundleName</key>\n"
"  <string>delve</string>\n"
"  <key>CFBundleVersion</key>\n"
"  <string>1.0</string>\n"
"  <key>SecTaskAccess</key>\n"
"  <array>\n"
"    <string>allowed</string>\n"
"    <string>debug</string>\n"
"  </array>\n"
"</dict>\n"
"</plist>\n";

kern_return_t
acquire_mach_task(int tid,
		mach_port_name_t *task,
		mach_port_t *port_set,
		mach_port_t *exception_port,
		mach_port_t *notification_port)
{
	kern_return_t kret;
	mach_port_t prev_not;
	mach_port_t self = mach_task_self();

	kret = task_for_pid(self, tid, task);
	if (kret != KERN_SUCCESS) return kret;

	// Allocate exception port.
	kret = mach_port_allocate(self, MACH_PORT_RIGHT_RECEIVE, exception_port);
	if (kret != KERN_SUCCESS) return kret;

	kret = mach_port_insert_right(self, *exception_port, *exception_port, MACH_MSG_TYPE_MAKE_SEND);
	if (kret != KERN_SUCCESS) return kret;

	kret = task_set_exception_ports(*task, EXC_MASK_BREAKPOINT|EXC_MASK_SOFTWARE, *exception_port,
			EXCEPTION_DEFAULT, THREAD_STATE_NONE);
	if (kret != KERN_SUCCESS) return kret;

	// Allocate notification port to alert of when the process dies.
	kret = mach_port_allocate(self, MACH_PORT_RIGHT_RECEIVE, notification_port);
	if (kret != KERN_SUCCESS) return kret;

	kret = mach_port_insert_right(self, *notification_port, *notification_port, MACH_MSG_TYPE_MAKE_SEND);
	if (kret != KERN_SUCCESS) return kret;

	kret = mach_port_request_notification(self, *task, MACH_NOTIFY_DEAD_NAME, 0, *notification_port,
			MACH_MSG_TYPE_MAKE_SEND_ONCE, &prev_not);
	if (kret != KERN_SUCCESS) return kret;

	// Create port set.
	kret = mach_port_allocate(self, MACH_PORT_RIGHT_PORT_SET, port_set);
	if (kret != KERN_SUCCESS) return kret;

	// Move exception and notification ports to port set.
	kret = mach_port_move_member(self, *exception_port, *port_set);
	if (kret != KERN_SUCCESS) return kret;

	return mach_port_move_member(self, *notification_port, *port_set);
}

char *
find_executable(int pid) {
	static char pathbuf[PATH_MAX];
	proc_pidpath(pid, pathbuf, PATH_MAX);
	return pathbuf;
}

kern_return_t
get_threads(task_t task, void *slice) {
	kern_return_t kret;
	thread_act_array_t list;
	mach_msg_type_number_t count;

	kret = task_threads(task, &list, &count);
	if (kret != KERN_SUCCESS) {
		return kret;
	}

	memcpy(slice, (void*)list, count*sizeof(list[0]));

	kret = vm_deallocate(mach_task_self(), (vm_address_t) list, count * sizeof(list[0]));
	if (kret != KERN_SUCCESS) return kret;

	return (kern_return_t)0;
}

int
thread_count(task_t task) {
	kern_return_t kret;
	thread_act_array_t list;
	mach_msg_type_number_t count;

	kret = task_threads(task, &list, &count);
	if (kret != KERN_SUCCESS) return -1;

	kret = vm_deallocate(mach_task_self(), (vm_address_t) list, count * sizeof(list[0]));
	if (kret != KERN_SUCCESS) return -1;

	return count;
}

mach_port_t
mach_port_wait(mach_port_t port_set) {
	kern_return_t kret;
	thread_act_t thread;
	union
	{
		mach_msg_header_t hdr;
		char data[256];
	} msg;

	// Wait for mach msg.
	kret = mach_msg(&msg.hdr, MACH_RCV_MSG|MACH_RCV_INTERRUPT,
			0, sizeof(msg.data), port_set, 0, MACH_PORT_NULL);
	if (kret == MACH_RCV_INTERRUPTED) return kret;
	if (kret != MACH_MSG_SUCCESS) return 0;

	mach_msg_body_t *bod = (mach_msg_body_t*)(&msg.hdr + 1);
	mach_msg_port_descriptor_t *desc = (mach_msg_port_descriptor_t *)(bod + 1);
	thread = desc[0].name;

	switch (msg.hdr.msgh_id) {
		case 2401: // Exception
			kret = thread_suspend(thread);
			if (kret != KERN_SUCCESS) return 0;

			// Send our reply back so the kernel knows this exception has been handled.
			mig_reply_error_t reply;
			mach_msg_header_t *rh = &reply.Head;
			rh->msgh_bits = MACH_MSGH_BITS(MACH_MSGH_BITS_REMOTE(msg.hdr.msgh_bits), 0);
			rh->msgh_remote_port = msg.hdr.msgh_remote_port;
			rh->msgh_size = (mach_msg_size_t) sizeof(mig_reply_error_t);
			rh->msgh_local_port = MACH_PORT_NULL;
			rh->msgh_id = msg.hdr.msgh_id + 100;

			reply.NDR = NDR_record;
			reply.RetCode = KERN_SUCCESS;

			kret = mach_msg(&reply.Head, MACH_SEND_MSG|MACH_SEND_INTERRUPT, rh->msgh_size, 0,
					MACH_PORT_NULL, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);

			if (kret != MACH_MSG_SUCCESS) return 0;
			break;

		case 72: // Death
			return msg.hdr.msgh_local_port;
	}

	return thread;
}
