use actix_web::{App, HttpServer};

mod config;
mod sysinfo;
mod online_status;
mod network_members;
mod network_join;
mod network_leave;
mod route_list;
mod member_online_status;
mod endpoint_list;
use endpoint_list::get_all_endpoints_handler;
mod network_member_list;
use network_member_list::get_all_network_members_handler;

use sysinfo::report_sysinfo;
use network_members::get_network_members;
use network_join::join_network;
use network_leave::leave_network;
use route_list::get_route_list;
use member_online_status::update_member_online_status;
use online_status::update_online_status;

const SERVER_IP: &str = "0.0.0.0";
const SERVER_PORT: u16 = 1001;

#[actix_web::main]
async fn main() -> std::io::Result<()> {
    let server_ip = SERVER_IP;
    let port = SERVER_PORT;
    println!("Server started, listening on port: {}:{}", server_ip, port);
    
    HttpServer::new(|| {
        App::new()
            .service(report_sysinfo)
            .service(get_network_members)
            .service(join_network)
            .service(leave_network)
            .service(get_route_list)
            .service(update_member_online_status)
            .service(update_online_status)
            .service(get_all_endpoints_handler)
            .service(get_all_network_members_handler)
    })
    .bind(format!("{}:{}", server_ip, port))?
    .run()
    .await
}