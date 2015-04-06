package proctl

import (
    "C"
    "fmt"
)

func setHardwareBreakpoint(reg, tid int, addr uint64) error {
    return fmt.Errorf("setHardwareBreakpoint not implemented on windows")
}

func clearHardwareBreakpoint(reg, tid int) error {
    return fmt.Errorf("clearHardwareBreakpoint not implemented on windows")
}
