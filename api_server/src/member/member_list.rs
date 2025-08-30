use actix_web::{get, web, HttpResponse, Responder};
use serde::{Deserialize, Serialize};
use chrono::Utc;

use crate::network::{
    MemberInfo,
    NETWORK_CONFIG,
};

#[derive(Debug, Deserialize, Serialize)]
struct SuccessResponse {
    status: String,
    data: Vec<MemberInfo>,
}

#[get("api/v1/networks/{network_id}/members/{member_id}")]
pub async fn get_network_members(
    path: web::Path<(String, String)>,
) -> impl Responder {
    let (network_id, member_id) = path.into_inner();

    let mut config = match NETWORK_CONFIG.lock() {
        Ok(guard) => guard,
        Err(_) => {
            return HttpResponse::InternalServerError().body("Failed to acquire member config lock");
        }
    };

    let network = config.iter_mut().find(|v| v.basic_info.id == network_id);
    match network {
        Some(network) => {
            // 更新成员在线时间
            let member = network.member_info.iter_mut().find(|m| m.id == member_id);
            if member.is_none() {
                return HttpResponse::NotFound().body("Member not found");
            }
            let member = member.unwrap();
            member.last_seen = Utc::now().to_string();

            // 返回网络成员
            return HttpResponse::Ok().json(SuccessResponse {
                status: "success".to_string(),
                data: network.member_info.clone(),
            });
        }
        None => {
            return HttpResponse::InternalServerError().body("Network ID not found");
        }
    }
}