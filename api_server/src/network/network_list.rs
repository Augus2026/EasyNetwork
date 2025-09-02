use actix_web::{get, HttpResponse, Responder};
use serde::{Serialize, Deserialize};

use crate::network::{
    NetworkConfig,
    NETWORK_CONFIG,
};

#[derive(Debug, Clone, Deserialize, Serialize)]
struct SuccessResponse {
    status: String,
    data: Vec<NetworkConfig>,
}

#[get("/api/v1/networks")]
pub async fn get_networks() -> impl Responder {
    let mut config = match NETWORK_CONFIG.lock() {
        Ok(config) => config,
        Err(_) => {
            return HttpResponse::InternalServerError().body("Failed to acquire network config lock");
        }
    };

    // 修改设备数量
    for network in config.iter_mut() {
        let actual_device_count = network.member_info.len().to_string();
        network.basic_info.devices = actual_device_count;
    }
    
    HttpResponse::Ok().json(SuccessResponse {
        status: "success".to_string(),
        data: config.clone(),
    })
}