package proctl

import (
    "fmt"

    sys "golang.org/x/sys/windows"
    "debug/pe"
    "sync"
    "os"
)

type OSProcessDetails struct {
}

func (dbp *DebuggedProcess) Halt() (error) {
    return fmt.Errorf("KW Halt() not implemented on windows")
}

func wait (pid, options int) (int, *sys.WaitStatus, error) {
    return 0, nil, fmt.Errorf("KWA wait() not implemented on windows")
}

func trapWait(dbp *DebuggedProcess, pid int) (*ThreadContext, *BreakPoint, error) {
    return nil, nil, fmt.Errorf("KWA trapWait() not implemented on windows")
}

func (dbp *DebuggedProcess) findExecutable() (*pe.File, error) {
    return nil, fmt.Errorf("KWA findExecutable() not implemented on windows")
}

func (dbp *DebuggedProcess) parseDebugFrame(exe *pe.File, wg *sync.WaitGroup) {
    defer wg.Done()

    fmt.Println("KWA parseDebugFrame() not implemented on Windows")
    os.Exit(1)
}

func (dbp *DebuggedProcess) obtainGoSymbols(exe *pe.File, wg *sync.WaitGroup) {
    defer wg.Done()

    fmt.Println("KWA obtainGoSymbols() not implemented on Windows")
}

func (dbp *DebuggedProcess) parseDebugLineInfo(exe *pe.File, wg *sync.WaitGroup) {
    defer wg.Done()

    fmt.Println("KWA parseDebugLineInfo() not implemented on windows")
    os.Exit(1)
}

func (dbp *DebuggedProcess) updateThreadList() error {
    return fmt.Errorf("KWA updateThreadList() not implemented on windows")
}