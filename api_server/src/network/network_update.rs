use actix_web::{put, web, HttpResponse, Responder};
use serde::{Deserialize, Serialize};

use crate::network::{
    BasicInfo,
    RouteInfo,
    DhcpInfo,
    DnsInfo,
    ServerInfo,
    NETWORK_CONFIG,
};

#[derive(Debug, Serialize, Deserialize)]
pub struct UpdateNetworkRequest {
    pub basic_info: BasicInfo,
    pub route_info: Vec<RouteInfo>,
    pub dhcp_info: DhcpInfo,
    pub dns_info: DnsInfo,
    pub server_info: ServerInfo,
}

#[put("/api/v1/networks/{id}")]
pub async fn network_update(
    path: web::Path<String>,
    body: web::Json<UpdateNetworkRequest>,
) -> impl Responder {
    let id = path.into_inner();
    let mut config = match NETWORK_CONFIG.lock() {
        Ok(mut config) => config,
        Err(_) => {
            return HttpResponse::InternalServerError().body("Failed to acquire network config lock");
        }
    };

    // 更新各个字段
    if let Some(network) = config.iter_mut().find(|net| net.basic_info.id == id) {
        network.basic_info = body.basic_info.clone();
        network.route_info = body.route_info.clone();
        network.dhcp_info = body.dhcp_info.clone();
        network.dns_info = body.dns_info.clone();
        network.server_info = body.server_info.clone();
        HttpResponse::Ok().json("Network updated successfully")
    } else {
        HttpResponse::NotFound().body("Network not found")
    }
}