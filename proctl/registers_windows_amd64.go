package proctl

import (
    "fmt"
)

type Regs struct {
}

func (r *Regs) PC() uint64 {
    fmt.Println("KWA PC() not implemented on windows")
    return 0
}

func (r *Regs) SP() uint64 {
    fmt.Println("KWA SP() not implemented on windows")
    return 0
}

func (r *Regs) SetPC(thread *ThreadContext, pc uint64) error {
    return fmt.Errorf("KWA SetPC() not implemented on windows")
}

func registers(thread *ThreadContext) (Registers, error) {
    return nil, fmt.Errorf("KWA registers() not implemented on windows")
}
