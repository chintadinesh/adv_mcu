module capture_timer(
    capture_gate,
    capture_complete,
    timer_enable,
    interrupt_out);

    capture_gate,
    timer_enable,
    capture_complete,
    interrupt_out);

    input [31:0] load;
    input capture_gate;
    input timer_enable;

    output [31:0] Cap_Timer_Out;
    output interrupt_out;

    reg [31:0] slv_reg0;
    reg [31:0] slv_reg1;
    reg [31:0] slv_reg2;
    reg [31:0] slv_reg3;


    slv_reg0[0] = capture_gate; // CDMA interrupt out signal
                                // to the GIC. Used to halt counter.
    slv_reg0[1] = 1'b0;
    slv_reg0[2] = 1'b0;
    slv_reg0[3] = 1'b0;
    slv_reg0[4] = capture_complete; // Flag to indicate that the
                                    // capture is complete

    slv_reg0[5] = 1'b0;
    slv_reg0[6] = 1'b0;
    slv_reg0[7] = timer_enable;         // Timer enable signal
    slv_reg0[31:8] = {16'hBEAD,8'h0};   // Debug


    assign interupt_out = slv_reg1[0]; // Active high asserts an
                                        // interrupt to the GIC.

    assign timer_enable = slv_reg1[1]; // Active high enables
                                        // capture timer to count.

    slv_reg1[31:2] = {16'hFEED,14'h0}; // Debug

    slv_reg2[31:0] = Cap_Timer_Out[31:0];

    slv_reg3[2:0] = state[2:0]; // This is the current state of
                                // the timer counter state machine.
    slv_reg3[3] = 1'b0;
    slv_reg3[31:4] = 28'h5555_CAB; // Debug


endmodule
