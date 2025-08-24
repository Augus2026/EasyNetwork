use actix_web::{get, web, HttpResponse, Responder};
use serde::{Deserialize, Serialize};

use crate::member::{
    MemberInfo,
    MEMBER_CONFIG,
};

#[derive(Debug, Deserialize, Serialize)]
struct SuccessResponse {
    status: String,
    count: u32,
    data: Vec<MemberInfo>,
}

#[get("api/v1/networks/{network_id}/members")]
pub async fn get_network_members(
    path: web::Path<String>,
) -> impl Responder {
    let network_id = path.into_inner();

    let config = match MEMBER_CONFIG.lock() {
        Ok(guard) => guard,
        Err(_) => {
            return HttpResponse::InternalServerError().body("Failed to acquire member config lock");
        }
    };

    let members = config.get(&network_id).cloned();
    match members {
        Some(m) => {
            return HttpResponse::Ok().json(SuccessResponse {
                status: "success".to_string(),
                count: m.len() as u32,
                data: m,
            });
        }
        None => {
            return HttpResponse::NotFound().body("No member found for this id");
        }
    }
}