package proctl

import (
    "fmt"
)

type OSSpecificDetails struct {
    registers uint64
}

func (t *ThreadContext) Halt() error {
    return fmt.Errorf("KWA Halt() not implemented on windows")
}

func (t *ThreadContext) blocked() bool {
    fmt.Println("KWA ThreadContext.blocked() not implemented on windows")
    return false
}

func (t *ThreadContext) resume() error {
    return fmt.Errorf("KWA resume() not implemented on windows")
}

func (t *ThreadContext) singleStep() error {
    return fmt.Errorf("KWA singleStep() not implemented on windows")
}

func readMemory(thread *ThreadContext, addr uintptr, data []byte) (int, error) {
    return 0, fmt.Errorf("KWA readMemory() not implemented on windows")
}

func writeMemory(thread *ThreadContext, addr uintptr, data []byte) (int, error) {
    return 0, fmt.Errorf("KWA writeMemory() not implemented on windows")
}

func (thread *ThreadContext) saveRegisters() error {
    return fmt.Errorf("KWA saveRegisters() not implemented on windows")
}

func (thread *ThreadContext) restoreRegisters() error {
    return fmt.Errorf("KWA restoreRegisters() not implemented on windows")
}