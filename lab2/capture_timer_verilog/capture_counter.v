module capture_counter(
    counter,
    state,
    capture_complete,

    clk,
    reset,
    timer_enable,
    capture_gate);

    input clk;
    input reset;
    input capture_gate;
    input timer_enable;

    output reg [31:0] counter;
    output reg [2:0] state;
    output reg capture_complete;
    

    parameter LOAD = 32'h1; // Used to load an offset values

    parameter RESET = 3'b000;
    parameter COUNT = 3'b010;
    parameter WAIT = 3'b011;
    parameter IDLE = 3'b100;

    initial begin
        counter <= LOAD;
        state <= RESET;
        capture_complete <= 0;
        end

    always @(posedge clk) begin
        if (reset) begin
            state <= RESET;
            capture_complete <= 0;
            counter <= 32'b0;
            end
        else begin
            case(state)
                (RESET): begin
                    capture_complete <= 0;

                    if(timer_enable) begin
                        state <= COUNT;
                        end
                    else begin
                        state <= RESET;
                        end
                    end
                (COUNT): begin
                    if(!capture_gate) begin
                        counter <= counter + 1;
                        state <= COUNT;
                        end
                    else if(capture_gate) begin
                        state <= WAIT;
                        end
                    else if(!timer_enable) begin
                        state <= WAIT;
                        end
                    end
                (WAIT): begin
                    capture_complete <= 1;

                    if(timer_enable) begin
                        state <= WAIT;
                        end
                    else begin
                        state <= IDLE;
                        end
                    end
                (IDLE): begin
                    capture_complete <= 0;
                    counter <= LOAD;

                    if(!timer_enable) begin
                        state <= IDLE;
                        end
                    else begin
                        state <= COUNT;
                        end
                    end
                default: begin
                        counter <= counter;
                        state <= state;
                        capture_complete <= capture_complete;
                        end
                endcase
            end
        end

    endmodule

