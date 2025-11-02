// nostdlib halt macro for Horizon
// (c) 2025- HorizonOS Project
//

#define K_ASSERT(_e, _msg) do \
    { if (!_e) { __asm__ volatile("hlt"); }} while (0);