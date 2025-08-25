use actix_web::{post, web, HttpResponse, Responder};
use serde::{Deserialize, Serialize};
use chrono::Utc;

use crate::network::{
    MemberInfo,
    NETWORK_CONFIG,
};

#[derive(Debug, Deserialize, Serialize)]
struct SuccessResponse {
    status: String,
}

#[post("/api/v1/networks/{network_id}/members/{member_id}/online_status")]
async fn update_member_online_status(
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
            let member = network.member_info.iter_mut().find(|m| m.id == member_id);
            if member.is_none() {
                return HttpResponse::NotFound().body("Member not found");
            }
            let member = member.unwrap();
            member.last_seen = Utc::now().to_string();
            return HttpResponse::Ok().json(SuccessResponse {
                status: "success".to_string(),
            });
        }
        None => {
            return HttpResponse::InternalServerError().body("Network ID not found");
        }
    }
}
