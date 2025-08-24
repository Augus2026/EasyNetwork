use actix_web::{get, HttpResponse, Responder};

use crate::network::{
    NETWORK_CONFIG,
};

#[get("/api/v1/networks")]
pub async fn get_networks() -> impl Responder {
    let config = match NETWORK_CONFIG.lock() {
        Ok(config) => config,
        Err(_) => {
            return HttpResponse::InternalServerError().body("Failed to acquire network config lock");
        }
    };

    let networks = config.iter() 
            .map(|v| v.basic_info.clone())
            .collect::<Vec<_>>();
    HttpResponse::Ok().json(networks)
}