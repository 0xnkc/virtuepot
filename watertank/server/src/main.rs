use std::env;
use env_logger;
use log::debug;

use tokio::net::TcpListener;
use tokio::sync::{watch, broadcast};

use watertank_simulation_server::utils::watertank::WaterTank;

use watertank_simulation_server::utils::simulation;

use watertank_simulation_server::utils::server;

use std::{thread, time};

#[tokio::main]
async fn main() {
    // Setup logging, run with RUST_LOG=debug to see log
    env_logger::init();

    // Address for gateway to connect to
    let gw_addr = "0.0.0.0:9977".to_string();

    // Address for the websocket to listen to
    let ws_addr = env::args()
        .nth(1)
        .unwrap_or_else(|| "0.0.0.0:7799".to_string());

    // Create a tank
    let tank = WaterTank {
        level: 1000.0,
        areal: 1000000.0,
        height: 2000.0,
        inflow: 20.0,
        inflow_mean: 20.0,
        inflow_stddev: 3.0,
        max_inflow: 40.0,
        outflow: 20.0,
        max_outflow: 40.0,
        set_level: 1000.0,
    };

    // Setup tcplistener to the gateway
    let gw_listener = TcpListener::bind(&gw_addr).await.unwrap();
    debug!("GW listening on: {}", gw_addr);

    // Setup tcplistener for the websocket
    let ws_listener = TcpListener::bind(&ws_addr).await.unwrap();
    debug!("WS listening on: {}", ws_addr);

    // Create a few channels to talk across threads
    let (txout, rxout) = watch::channel(tank);
    let (txin, rxin) = broadcast::channel(2);

    // Start and run the listeners and simulation.
    let t = server::listen_tcp(gw_listener, rxout.clone(), txin.clone());
    let ws = server::listen_ws(ws_listener, rxout.clone());
    let r = simulation::run_simulation(txout, rxin, tank);
    r.await;
    ws.await;
    t.await;
    
    // run forever, but with a small delay so we dont overheat our pc.
    let delay = time::Duration::from_millis(20);
    loop {
        thread::sleep(delay);
    }
}