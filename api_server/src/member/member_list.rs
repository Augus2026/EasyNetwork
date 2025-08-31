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

#[get("api/v1/networks/{network_id}/members")]
pub async fn get_network_members(
    path: web::Path<String>,
) -> impl Responder {
    let (network_id) = path.into_inner();

    let mut config = match NETWORK_CONFIG.lock() {
        Ok(guard) => guard,
        Err(_) => {
            return HttpResponse::InternalServerError().body("Failed to acquire member config lock");
        }
    };

    let network = config.iter_mut().find(|v| v.basic_info.id == network_id);
    match network {
        Some(network) => {
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