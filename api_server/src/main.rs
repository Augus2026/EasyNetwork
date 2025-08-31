use actix_web::{App, HttpServer};

mod endpoint;
mod member;
mod network;

const SERVER_IP: &str = "0.0.0.0";
const SERVER_PORT: u16 = 8000;

#[actix_web::main]
async fn main() -> std::io::Result<()> {
    let server_ip = SERVER_IP;
    let port = SERVER_PORT;
    println!("Server started, listening on port: {}:{}", server_ip, port);
    
    HttpServer::new(|| {
        App::new()
            // endpoint
            .service(endpoint::endpoint_sysinfo::report_sysinfo)
            .service(endpoint::endpoint_list::get_all_endpoints)
            .service(endpoint::endpoint_online_status::update_online_status)
            //member
            .service(member::member_list::get_network_members)
            .service(member::member_status_list::get_network_member_status)
            .service(member::member_join::member_join)
            .service(member::member_leave::member_leave)
            // network
            .service(network::network_add::network_add)
            .service(network::network_del::network_del)
            .service(network::network_list::get_networks)
            .service(network::network_update::network_update)
            .service(network::network_route_list::network_route_list)
    })
    .bind(format!("{}:{}", server_ip, port))?
    .run()
    .await
}