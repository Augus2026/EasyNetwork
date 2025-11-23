use actix_web::{App, HttpServer, middleware::Logger, web, HttpResponse};
use actix_files::Files;
use actix_cors::Cors;
use std::env;
use std::fs::OpenOptions;
use std::io::Write;
use env_logger::{Builder, Target};
use log::{info, warn, error};

mod device;
mod member;
mod network;
mod database;

const DEFAULT_SERVER_IP: &str = "localhost";
const DEFAULT_SERVER_PORT: u16 = 1000;
const DEFAULT_DASHBOARD_PATH: &str = "./dashboard/build/web";

#[actix_web::main]
async fn main() -> std::io::Result<()> {
    let mut server_ip = DEFAULT_SERVER_IP.to_string();
    let mut server_port = DEFAULT_SERVER_PORT;
    let mut dashboard_path = DEFAULT_DASHBOARD_PATH.to_string();
    let mut db_path = "./config.db".to_string();
    let mut log_path = "./api_server.log".to_string();  // 默认日志路径

    // 解析命令行参数
    let args: Vec<String> = env::args().collect();
    for i in 1..args.len() {
        if args[i] == "--server-ip" && i + 1 < args.len() {
            server_ip = args[i + 1].clone();
            break;
        }
    }
    for i in 1..args.len() {
        if args[i] == "--server-port" && i + 1 < args.len() {
            server_port = args[i + 1].parse().unwrap();
            break;
        }
    }
    for i in 1..args.len() {
        if args[i] == "--dashboard-path" && i + 1 < args.len() {
            dashboard_path = args[i + 1].clone();
            break;
        }
    }
    for i in 1..args.len() {
        if args[i] == "--db-path" && i + 1 < args.len() {
            db_path = args[i + 1].clone();
            break;
        }
    }
    for i in 1..args.len() {
        if args[i] == "--log-path" && i + 1 < args.len() {
            log_path = args[i + 1].clone();
            break;
        }
    }

    let log_file = OpenOptions::new()
        .create(true)
        .append(true)
        .open(log_path.clone())
        .expect("unable to open log file");

    Builder::new()
        .target(Target::Pipe(Box::new(log_file)))
        .filter_level(log::LevelFilter::Info)
        .init();

    info!("Server started, listening on port: {}:{} dashboard path: {}, log path: {}", 
             server_ip, server_port, dashboard_path, log_path);

    // 初始化数据库
    if let Err(e) = database::init_database(db_path) {
        error!("Failed to initialize database: {}", e);
        return Err(std::io::Error::new(std::io::ErrorKind::Other, e.to_string()));
    }

    // 加载所有配置
    if let Err(e) = database::load_all_config().await {
        error!("Failed to load device configuration: {}", e);
    } else {
        info!("Device configuration loaded successfully");
    }

    // 启动定时保存任务
    let auto_save_handle = tokio::spawn(database::start_auto_save());

    let server = HttpServer::new(move || {
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
                    .append_header(("Location", "/dashboard"))
                    .finish()
            })))
            // 静态文件服务
            .service(Files::new("/dashboard", &dashboard_path)
                .index_file("index.html")
                .show_files_listing())
            // endpoint
            .service(device::device_sysinfo::report_device_sysinfo)
            .service(device::device_list::get_all_devices)
            .service(device::device_online_status::update_online_status)
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
    .await;

    info!("Server stopped");
    // 取消定时保存任务
    auto_save_handle.abort();
    // 保存所有配置
    database::save_all_config().await;

    Ok(())
}