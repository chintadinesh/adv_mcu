
module capture_counter_tb;

    wire [31:0] counter;
    wire [2:0] state;
    wire capture_complete;

    reg clk;
    reg reset;
    reg timer_enable;
    reg capture_gate;

    capture_counter dut(
       .counter     (counter),
       .state       (state),
        .capture_complete(capture_complete),

       .clk         (clk),
       .reset       (reset),
       .timer_enable(timer_enable),
       .capture_gate(capture_gate)
    );
    
    initial begin
        clk <= 0;
        forever begin
            #1 clk = ~clk;
            end
        end


    initial begin

        $monitor("time = %3d, state = %3b, reset= %b, timer_enable = %b, capture_gate = %b, capture_complete = %b \n",
            $time,
            state,
            reset,
            timer_enable,
            capture_gate,
            capture_complete);

        // create the test bench sequence here
        reset <= 1'b1;
        timer_enable  <= 0;
        capture_gate <= 0;

        #2
        reset = 0;

        // no active inputs
        #2
        timer_enable = 1'b0;
        capture_gate = 1'b0;
        $display("time = %3d, counter = %d\n", $time, counter);

        // capture gate on, in RESET state. Should do nothing
        #2
        timer_enable = 1'b0;
        capture_gate = 1'b1;
        $display("time = %3d, counter = %d\n", $time, counter);

        // deasserting gate.
        #2
        timer_enable = 1'b0;
        capture_gate = 1'b0;
        $display("time = %3d, counter = %d\n", $time, counter);
         
        // enabling the timer. This should start the counter
        #2
        timer_enable = 1'b1;
        capture_gate = 1'b0;
        $display("time = %3d, counter = %d\n", $time, counter);

        // wait for counter to increment
        #40

        // assert capture. state should change to WAIT.
        #2
        timer_enable = 1'b1;
        capture_gate = 1'b1;
        $display("time = %3d, counter = %d\n", $time, counter);

        // capture gate should have no effect
        // deasserting timer enable should keep the sate in WAIT.
        #2
        timer_enable = 1'b1;
        capture_gate = 1'b1;
        $display("time = %3d, counter = %d\n", $time, counter);

        // capture gate should have no effect
        // reasserting timer enable should keep the sate in WAIT.
        #2
        timer_enable = 1'b1;
        capture_gate = 1'b1;
        $display("time = %3d, counter = %d\n", $time, counter);

        // to make sure that capture gete has no effect
        #4

        // deassert timer_enable to move to IDLE state
        #2
        timer_enable = 1'b0;
        capture_gate = 1'b0;
        $display("time = %3d, counter = %d\n", $time, counter);

        end

endmodule
