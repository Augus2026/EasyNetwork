use actix_web::{post, web, HttpResponse, Responder};
use serde::{Deserialize, Serialize};

use crate::network::{
    BasicInfo,
    RouteInfo,
    DhcpInfo,
    DnsInfo,
    ServerInfo,
    NetworkConfig,
    NETWORK_CONFIG,
};

#[derive(Debug, Serialize, Deserialize)]
pub struct AddNetworkRequest {
    pub id: String,
    pub name: String,
    pub desc: String,
    pub devices: String,
    pub created: String,
    pub is_private: bool,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct SuccessResponse {
    pub status: String,
    pub data: NetworkConfig,
}

#[post("/api/v1/networks")]
pub async fn network_add(
    body: web::Json<AddNetworkRequest>
) -> impl Responder {
    let mut network_config = match NETWORK_CONFIG.lock() {
        Ok(network_config) => network_config,
        Err(_) => {
            return HttpResponse::InternalServerError().body("Failed to acquire network config lock");
        }
    };

    let id = body.id.clone();
    let network = network_config.iter().find(|v| v.basic_info.id == id);
    if network.is_some() {
        return HttpResponse::InternalServerError().body("Network ID already exists");
    }

    let new_network = NetworkConfig {
        basic_info: BasicInfo {
            id: body.id.clone(),
            name: body.name.clone(),
            desc: body.desc.clone(),
            devices: body.devices.clone(),
            created: body.created.clone(),
            is_private: body.is_private,
        },
        route_info: vec![RouteInfo {
            dest: "192.168.1.0".to_string(),
            netmask: "255.255.255.0".to_string(),
            gateway: "192.168.1.1".to_string(),
            metric: 100,
        }],
        dhcp_info: DhcpInfo {
            alloc_type: "easy".to_string(),
            range_start: "192.168.1.100".to_string(),
            range_end: "192.168.1.200".to_string(),
            selected_range: "192.168.196.*".to_string(),
        },
        dns_info: DnsInfo {
            search_domain: "easy_network.com".to_string(),
            server_address: "default.easy_network.com".to_string(),
        },
        server_info: ServerInfo {
            server_address: "192.168.1.1".to_string(),
            server_port: "8080".to_string(),
        },
    };
    network_config.push(new_network.clone());
    
    HttpResponse::Ok().json(
        SuccessResponse {
            status: "success".to_string(),
            data: new_network.clone(),
        }
    )
}