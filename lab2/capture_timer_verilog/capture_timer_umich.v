module timer(
  PCLK,
  PENABLE,
  PSEL,
  PRESETN,
  PWRITE,
  PREADY,
  PSLVERR,
  PADDR,
  PWDATA,
  PRDATA,
  TCLK,
  test_out);

// APB Bus Interface
input PCLK,PENABLE, PSEL, PRESETN, PWRITE;
input  [31:0] PWDATA;
input  [3:0] PADDR;
output [31:0] PRDATA;
output PREADY, PSLVERR;

// Timer Interface
input TCLK;

// Test Interface
output [3:0] test_out;

// Clock Domain synchronization
reg PREADY;
reg [1:0] fsm; //state machine register for pclk domain
reg fsm_tclk;  // state machine register for tclk domain
reg [31:0]   PRDATA;

// Timer
reg [31:0] Counter;
reg [31:0] Load;
reg TimerEn;   // Timer enable
reg LoadEnReg; // Register Load enable

// Handshaking signals
reg reg_load_en0, reg_load_en1, reg_load_en2;
reg reg_load_ack0, reg_load_ack1, reg_load_ack2;

// Test Outputs
wire [3:0] test_out;
assign test_out[0] = 1'b0;
assign test_out[1] = 1'b0;
assign test_out[2] = 1'b0;
assign test_out[3] = 1'b0;

assign PSLVERR = 1'b0;         

// Handshaking signal PCLK signals to TCLK
always@(posedge PCLK or negedge PRESETN)
if(~PRESETN)
  begin
    reg_load_en1 <= 1'b0;
    reg_load_en2 <= 1'b0;
  end
else
  begin
    reg_load_en1 <= reg_load_en0;
    reg_load_en2 <= reg_load_en1;
  end
  
// Handshaking signal TCLK signals to PCLK
always@(posedge TCLK or negedge PRESETN)
if(~PRESETN)
  begin
    reg_load_ack1 <= 1'b0;
    reg_load_ack2 <= 1'b0;
  end
else
  begin
    reg_load_ack1 <= reg_load_ack0;
    reg_load_ack2 <= reg_load_ack1;
  end
  
// Handle APB Bus read and writes and inform TCLK clock domain
always@(posedge PCLK or negedge PRESETN)
if(~PRESETN)
  begin
      fsm <= 2'b00;
      PREADY <= 1'b1;
      reg_load_en0 <= 1'b0;
      end
else
  begin
  case (fsm)
     2'b00 :  begin
                  if (~PSEL)
                        begin
                          // not for us
                          fsm <= 2'b00;
                        end
                  else        
                        begin
                          fsm <= 2'b01; // advance to next state
                          PREADY <= 1'b0; // signal we are not ready
                          reg_load_en0 <= 1'b1; // inform other clock domain
                        end
               end

     2'b01 :  begin            
                if(reg_load_ack2 == 1'b1)
                  begin
                    fsm <= 2'b10;
                    reg_load_en0 <= 1'b0;
                  end
              end

     2'b10 :  begin            
                if(reg_load_ack2 == 1'b0)
                  begin
                    fsm <= 2'b11;
                    PREADY <= 1'b1; // we are ready
                  end
              end

     2'b11 :  begin            
                fsm <= 2'b00;
              end

    default : fsm <= 2'b00;
  endcase
end

always@(posedge TCLK or negedge PRESETN)
if(~PRESETN)
  begin
    fsm_tclk <= 1'b0;
    reg_load_ack0 <= 1'b0;
    LoadEnReg <= 1'b0;
    TimerEn <= 1'b0;
    Load <= 32'h00000000;
  end
else
  begin
    case (fsm_tclk)
      1'b0: begin
              if(reg_load_en2 == 1'b1) // signal from other clock domain
                begin
                  if(PWRITE)
                    begin // it's a write
                        case(PADDR[3:2])
                          2'b00: begin // Timer Load register
                                    Load <= PWDATA;
                                    LoadEnReg <= 1'b1;
                                 end
                          2'b01: begin // Timer Value, no write
                                    LoadEnReg <= 1'b0;
                                 end
                          2'b10: begin // Timer Control
                                    LoadEnReg <= 1'b0;
                                    TimerEn <= PWDATA[0];
                                 end
                          2'b11: begin // spare
                                    LoadEnReg <= 1'b0;
                                 end
                        endcase
                    end
                  else
                    begin // it's a read
                        case(PADDR[3:2])
                          2'b00: begin // Timer Load register
                                    PRDATA <= Load;
                                 end
                          2'b01: begin // Timer Value, no write
                                    PRDATA <= Counter;
                                 end
                          2'b10: begin // Timer Control
                                    PRDATA[31:1] <= 31'h00000000;
                                    PRDATA[0] <= TimerEn;
                                 end
                          2'b11: begin // spare
                                 end
                        endcase
                    end
                  fsm_tclk <= 1'b1; // advance state
                  reg_load_ack0 <= 1'b1; // signal other domain
                end
            end
      1'b1: begin
              if (reg_load_en2 == 1'b0)
                begin
                  fsm_tclk <= 1'b0;
                  reg_load_ack0 <= 1'b0;
                  LoadEnReg <= 1'b0;
                end
            end
      default: fsm_tclk <= 1'b0;
    endcase
  end

  
always@(posedge TCLK or negedge PRESETN)
if(~PRESETN)
  begin
    Counter <= 32'h00000000;
  end
else
  begin
    if(LoadEnReg == 1'b1)
      begin
        Counter <= 32'h00000000;
      end
    else if(TimerEn == 1'b1)
      begin
        if(Counter == Load)
          begin
            Counter <= 32'h00000000;
          end
        else
          begin
            Counter <= Counter + 1;
          end
      end
  end

endmodule
