use actix_web::{post, web, HttpResponse, Responder};
use serde::Deserialize;
use serde::Serialize;

use crate::network::{
    RouteInfo,
    NETWORK_CONFIG,
};

#[derive(Debug, Deserialize, Serialize)]
struct SuccessResponse {
    status: String,
    data: Vec<RouteInfo>,
}

#[post("/api/v1/networks/{id}/route_list")]
pub async fn network_route_list(
    path: web::Path<String>,
) -> impl Responder {
    let id = path.into_inner();
    let config = match NETWORK_CONFIG.lock() {
        Ok(config) => config,
        Err(_) => {
            return HttpResponse::InternalServerError().body("Failed to acquire network config lock");
        }
    };
    if let Some(network) = config.
        iter().
        find(|net| net.basic_info.id == id) {
        let route_list = network.route_info.clone();
        HttpResponse::Ok().json(SuccessResponse {
            status: "success".to_string(),
            data: route_list,
        })
    } else {
        HttpResponse::NotFound().body("Network not found")
    }
}
