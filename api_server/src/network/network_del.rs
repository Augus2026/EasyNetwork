use actix_web::{delete, web, HttpResponse, Responder};

use crate::network::{
    NETWORK_CONFIG,
};

#[delete("/api/v1/networks/{id}")]
pub async fn network_del(
    path: web::Path<String>,
) -> impl Responder {
    let id = path.into_inner();
    
    let mut network_config = match NETWORK_CONFIG.lock() {
        Ok(network_config) => network_config,
        Err(_) => {
            return HttpResponse::InternalServerError().body("Failed to acquire network config lock");
        }
    };

    let original_len = network_config.len();
    network_config.retain(|net| net.basic_info.id != id);
    
    if network_config.len() < original_len {
        HttpResponse::Ok().json("Network deleted successfully")
    } else {
        HttpResponse::NotFound().body("Network not found")
    }
}