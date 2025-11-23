use actix_web::{post, web, HttpResponse, Responder};
use serde::{Deserialize, Serialize};
use log::{info, error};

use crate::network::{
    MemberInfo,
    NETWORK_CONFIG,
};
#[derive(Debug, Deserialize, Serialize)]
struct SuccessResponse {
    status: String,
}

#[post("/api/v1/networks/{network_id}/members/{member_id}/leave")]
async fn member_leave(
    path: web::Path<(String, String)>,
) -> impl Responder {
    info!("member_leave {:?}", path);

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
            if let Some(index) = network.member_info.iter().position(|m| m.id == member_id) {
                network.member_info.remove(index);
                HttpResponse::Ok().json(SuccessResponse {
                    status: "success".to_string(),
                })
            } else {
                HttpResponse::NotFound().body("Member not found")
            }
        }
        None => {
            return HttpResponse::InternalServerError().body("Network ID not found");
        }
    }
}
