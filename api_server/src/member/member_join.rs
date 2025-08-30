use actix_web::{post, web, HttpRequest, HttpResponse, Responder};
use chrono::Utc;
use serde::{Deserialize, Serialize};
use std::sync::atomic::{AtomicU8, Ordering};

use crate::network::{
    BasicInfo,
    RouteInfo,
    DhcpInfo,
    DnsInfo,
    ServerInfo,
    MemberInfo,
    NETWORK_CONFIG,
};

#[derive(Debug, Serialize, Deserialize)]
pub struct MemberJoinRequest {
    pub uuid: String,
    pub network_id: String,
    pub version: String,
}

#[derive(Debug, Deserialize, Serialize)]
struct MemberJoinInfo {
    // basic info
    pub basic_info: BasicInfo,
    // route info
    pub route_info: Vec<RouteInfo>,
    // dhcp info
    pub dhcp_info: DhcpInfo,
    // dns info
    pub dns_info: DnsInfo,
    // server info
    pub server_info: ServerInfo,
    // member info
    pub member_info: MemberInfo,
}

#[derive(Debug, Deserialize, Serialize)]
struct SuccessResponse {
    status: String,
    data: MemberJoinInfo,
}

#[post("/api/v1/networks/{network_id}/join")]
async fn member_join(
    path: web::Path<String>,
    req: HttpRequest,
    body: web::Json<MemberJoinRequest>,
) -> impl Responder {
    println!("member_join path {:?} body {:?}", path, body);

    let network_id = path.into_inner();
    let mut config = match NETWORK_CONFIG.lock() {
        Ok(guard) => guard,
        Err(_) => {
            return HttpResponse::InternalServerError().body("Failed to acquire member config lock");
        }
    };

    let network = config.iter_mut().find(|v| v.basic_info.id == network_id);
    match network {
        Some(network) => {
            // 客户端版本
            let client_version = body.version.clone();
            // 客户端ip地址
            let client_ip = req.connection_info().peer_addr().unwrap_or("0.0.0.0").to_string();
            // 最新时间
            let last_seen = Utc::now().to_string();
            // 生成id
            let member_id = uuid::Uuid::new_v4().to_string();
            // 生成mac地址
            let mac_address = uuid::Uuid::new_v4().to_string();
            // 生成ip地址
            let alloc_type  = network.dhcp_info.alloc_type.clone();
            let ip4_address: String;
            let subnet_mask: String;
            match alloc_type.as_str() {
                "easy" => {
                    let range_start = 0;
                    let range_end = 255;
                    let ip_prefix = network.dhcp_info.selected_range.split('.').take(3).collect::<Vec<&str>>().join(".");
                    let ip_last_octet = range_start;
                    ip4_address = format!("{}.{}", ip_prefix, ip_last_octet);
                    subnet_mask = "255.255.255.0".to_string();
                }
                "advanced" => {
                    let range_start = network.dhcp_info.range_start.split('.').last().unwrap().parse::<u8>().unwrap();
                    let range_end = network.dhcp_info.range_end.split('.').last().unwrap().parse::<u8>().unwrap();
                    let ip_prefix = &network.dhcp_info.range_start[..network.dhcp_info.range_start.rfind('.').unwrap()];
                    let ip_last_octet = range_start;
                    ip4_address = format!("{}.{}", ip_prefix, ip_last_octet);
                    subnet_mask = "255.255.255.0".to_string();
                }
                _ => {
                    return HttpResponse::InternalServerError().body("Invalid alloc type");
                }
            }
            // 生成认证结果
            let auth = if network.basic_info.is_private { "false" } else { "true" };

            // 分配网络成员信息
            let member = MemberInfo {
                id: member_id.clone(),
                name: network.basic_info.name.clone(),
                desc: network.basic_info.desc.clone(),
                auth: auth.to_string(),
                mac_address: mac_address.clone(),
                ipv4_address: ip4_address.clone(),
                subnet_mask: subnet_mask.clone(),
                mtu: 1400,
                last_seen: last_seen.clone(),
                version: client_version,
                physical_ip: client_ip,
            };
            network.member_info.push(member.clone());

            HttpResponse::Ok().json(SuccessResponse {
                status: "success".to_string(),
                data: MemberJoinInfo {
                    basic_info: network.basic_info.clone(),
                    route_info: network.route_info.clone(),
                    dhcp_info: network.dhcp_info.clone(),
                    dns_info: network.dns_info.clone(),
                    server_info: network.server_info.clone(),
                    member_info: member.clone(),
                },
            })
        }
        None => {
            return HttpResponse::InternalServerError().body("Network ID not found");
        }
    }
}