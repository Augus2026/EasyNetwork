use actix_web::{App, HttpServer, middleware::Logger, web, HttpResponse};
use actix_files::Files;
use actix_cors::Cors;

mod endpoint;
mod member;
mod network;

const SERVER_IP: &str = "0.0.0.0";
const SERVER_PORT: u16 = 8000;
const DASHBOARD_PATH: &str = "./dashboard/build/web";

#[actix_web::main]
async fn main() -> std::io::Result<()> {
    // 初始化日志
    std::env::set_var("RUST_LOG", "actix_web=info");
    env_logger::init();

    let server_ip = SERVER_IP;
    let server_port = SERVER_PORT;
    println!("Server started, listening on port: {}:{}", server_ip, server_port);
    
    HttpServer::new(|| {
        App::new()
            // CORS中间件
            .wrap(
                Cors::default()
                    .allowed_origin("http://localhost:8000")
                    .allowed_origin("http://127.0.0.1:8000")
                    .allowed_methods(vec!["GET", "POST", "PUT", "DELETE"])
                    .allowed_headers(vec![actix_web::http::header::AUTHORIZATION, actix_web::http::header::ACCEPT])
                    .allowed_header(actix_web::http::header::CONTENT_TYPE)
                    .max_age(3600)
            )
            // logger中间件
            .wrap(Logger::default())
            // index.html
            .service(web::resource("/").route(web::get().to(|| async {
                HttpResponse::Found()
                    .append_header(("Location", "/easy_network"))
                    .finish()
            })))
            // 静态文件服务
            .service(Files::new("/easy_network", DASHBOARD_PATH)
                .index_file("index.html")
                .show_files_listing())
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
    .bind(format!("{}:{}", server_ip, server_port))?
    .run()
    .await
}