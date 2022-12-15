pub mod utils {
    
    pub mod watertank {
        use serde::{Serialize, Deserialize};
        use rand::prelude::*;
        use rand_distr::{Normal, Distribution};

        // How many cubic MM in one liter, Thousand liters in one cubic M. 
        const L_TO_CUBIC_MM: f32 = 1000000.0;
        /// A water tank struct
        /// There is flow into the water tank a size of the water tank and a level we want the water to be
        /// There is an areal of the water tank, it is a box watertank with a hight
        /// There is also a valve out of the water tank that controls the outflow
        /// 
        #[derive(Serialize, Deserialize, Debug, Copy, Clone)]
        pub struct WaterTank {
            pub level: f32,         // the water level of the tank mm. 
            pub inflow_mean: f32,   // the mean inflow if the tank l/s
            pub inflow_stddev: f32, // the stddev of inflow of the tank l/s
            pub inflow: f32,        // the inflow right now
            pub max_inflow: f32,
            pub areal: f32,         // the areal of the tank mm^2
            pub height: f32,        // the height of the tank mm
            pub outflow: f32,       // the outflow of the tank l/s
            pub max_outflow: f32,
            pub set_level: f32,     // the wanted level of the tank mm, Real value? or 4 - 20 mA?
        }
    
        impl WaterTank {
            fn _volume(&self) -> f32 {
                self.areal * self.height
            }
    
            pub fn update_level(&mut self, seconds_passed: f32) {
                // water volume of the tank = areal * level
                // change in volume = volume + (inflow - outflow) * seconds_passed
                let volume = (self.areal * self.level) + ((self.inflow - self.outflow) * seconds_passed * L_TO_CUBIC_MM );
                self.level = volume / self.areal;
            }
    
            pub fn update_inflow(&mut self) {
                let mut rng = thread_rng();
                let normal = Normal::new(self.inflow_mean, self.inflow_stddev).unwrap();
                let v = normal.sample(&mut rng);
                self.inflow = v;
            }
        }
    }

    pub mod simulation {
        use log::debug;
        use tokio::sync::{watch, broadcast};
        use tokio::time::{sleep, Duration};
        use crate::utils::watertank::WaterTank;
        use crate::utils::protocol::Payload;

        pub async fn run_simulation(
                    txout: watch::Sender<WaterTank>, 
                    mut rxin: broadcast::Receiver<Payload>, 
                    mut tank: WaterTank) 
            {
            tokio::spawn(async move {
                debug!("Starting simulation");
                loop {
                    // Wait so we dont run too fast
                    sleep(Duration::from_millis(300)).await;
        
                    // Get and update outflow control setpoint
                    let payload = rxin.recv().await.unwrap();
                    tank.outflow = (payload.outflow as f32 / 65535.0) as f32 * 40.0; // create helper function
        
        
                    tank.update_inflow();
                    tank.update_level(0.3);
                    
                    txout.send(tank).unwrap();
                    debug!("Tank: {:?}", tank);
                }
            });
        }
    }

    pub mod server {
        use log::debug;
        use tokio::net::{TcpListener, TcpStream};
        use tokio::time::{sleep, Duration};
        use tokio::sync::{watch, broadcast};
        use tokio::io::AsyncWriteExt;

        use futures_util::StreamExt;
        use futures::sink::SinkExt;
        use tokio_tungstenite::tungstenite::Message;

        use crate::utils::protocol;
        use crate::utils::protocol::{Payload, ReturnMessage};
        use crate::utils::watertank::WaterTank;

        pub async fn listen_ws(listener: TcpListener, rxout: watch::Receiver<WaterTank>) {
            tokio::spawn(async move {
                while let Ok((stream, _)) = listener.accept().await {
                    handle_ws(stream, rxout.clone()).await;
                }
            });
        }

        async fn handle_ws(stream: TcpStream, rxout: watch::Receiver<WaterTank>) {
            tokio::spawn(async move {
                let addr = stream.peer_addr().expect("connected streams should have a peer address");
                debug!("Peer address: {}", addr);
        
                let ws_stream = tokio_tungstenite::accept_async(stream)
                    .await
                    .expect("Error during the websocket handshake occurred");
        
                debug!("New WebSocket connection: {}", addr);
        
                let (mut write, _) = ws_stream.split();
        
                loop {
                    sleep(Duration::from_millis(500)).await;
                    let tank = *rxout.borrow();
                    let msg = Message::Text(serde_json::to_string(&tank).unwrap());
                    write.send(msg).await.unwrap();
                }
            });
        }

        pub async fn listen_tcp(listener: TcpListener, rxout: watch::Receiver<WaterTank>, txin: broadcast::Sender<Payload>) {
            tokio::spawn(async move {
                while let Ok((stream, addr)) = listener.accept().await {
                    debug!("New connection from {:?}", addr);
                    handle_gw(stream, rxout.clone(), txin.clone()).await;
                }
            });
        }

        async fn handle_gw(mut stream: TcpStream, rxout: watch::Receiver<WaterTank>, txin: broadcast::Sender<Payload>) {
    
            tokio::spawn(async move {
                debug!("Handle new connection");
        
                // In a loop, read data from the socket and write the data back.
                loop {
                    let tank = *rxout.borrow();
                    let tank_level = protocol::convert_f32_to_mobdus_u16(0.0, tank.height, tank.level);
                    let tank_inflow = protocol::convert_f32_to_mobdus_u16(0.0, tank.max_inflow, tank.inflow);

                    let message = ReturnMessage {
                        msg_type: String::from("input-register"),
                        address: 0,
                        tank_level: tank_level,
                        tank_inflow: tank_inflow,
                    };
        
                    let (mut reader, mut writer) = stream.split();
        
                    // read header length
                    let mut len = vec![0; 1];
                    match reader.peek(&mut len).await.unwrap() {
                        0 => {break},
                        _ => {},
                    };
        
                    let header = protocol::read_header(len, &mut reader).await;
        
                    // read payload
                    let payload = protocol::read_payload(header, &mut reader).await;
                    debug!("Payload {:?}", payload);
        
                    // send payload to simulation
                    txin.send(payload).unwrap();
        
                    let mut message = serde_json::to_string(&message).unwrap();
                    debug!("Sending {}", message);
                    message.push('\n');
                    writer.write_all(message.as_bytes()).await.unwrap();
                }
            });
        }

    }

    pub mod protocol {
        use serde::{Serialize, Deserialize};
        use tokio::net::tcp::ReadHalf;
        use tokio::io::AsyncReadExt;

        #[derive(Serialize, Deserialize, Debug, Copy, Clone)]
        pub struct Payload {
            pub outflow: u16,
            pub setpoint: u16,
        }

        #[derive(Serialize, Deserialize, Debug)]
        pub struct Header {
            pub len: i32,
            pub msg_type: String,
        }

        #[derive(Serialize, Deserialize, Debug)]
        pub struct ReturnMessage {
            pub msg_type: String, 
            pub address: i32,
            pub tank_level: u16,
            pub tank_inflow: u16,
        }

        pub fn convert_f32_to_mobdus_u16(_min: f32, max: f32, value: f32) -> u16{
            let max = 65535 as f32 / max;
            (value as f32 * max) as u16
        }

        pub async fn read_header(mut len: Vec<u8>, reader: &mut ReadHalf<'_>) -> Header {
            reader.read(&mut len).await.unwrap();
            // read header
            let mut header = vec![0; len[0] as usize];
            reader.read(&mut header).await.unwrap();
            let header_string = std::str::from_utf8(&header).unwrap();  
            serde_json::from_str(header_string).unwrap()
        }

        pub async fn read_payload(header: Header, reader: &mut ReadHalf<'_>) -> Payload {
            let mut payload = vec![0; header.len as usize];
            reader.read(&mut payload).await.unwrap();
            let payload_string = std::str::from_utf8(&payload).unwrap();
            serde_json::from_str(payload_string).unwrap()
        }
    }
}
