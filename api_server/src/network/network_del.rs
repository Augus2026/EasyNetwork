use actix_web::{delete, web, HttpResponse, Responder};
use serde::{Serialize, Deserialize};

use crate::network::{
    NETWORK_CONFIG,
};

#[derive(Debug, Serialize, Deserialize)]
pub struct SuccessResponse {
    pub status: String,
}

#[delete("/api/v1/networks/{network_id}")]
pub async fn network_del(
    path: web::Path<String>,
) -> impl Responder {
    let network_id = path.into_inner();
    
    let mut network_config = match NETWORK_CONFIG.lock() {
        Ok(network_config) => network_config,
        Err(_) => {
            return HttpResponse::InternalServerError().body("Failed to acquire network config lock");
        }
    };

    let original_len = network_config.len();
    network_config.retain(|net| net.basic_info.id != network_id);
    
    if network_config.len() < original_len {
        HttpResponse::Ok().json(SuccessResponse {
            status: "success".to_string(),
        })
    } else {
        HttpResponse::NotFound().body("Network not found")
    }
}