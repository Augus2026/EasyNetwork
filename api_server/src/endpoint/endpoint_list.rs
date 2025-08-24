use actix_web::{get, HttpResponse, Responder};
use serde::Serialize;

use crate::endpoint::{
    EndpointConfig,
    ENDPOINT_CONFIG,
};

#[derive(Debug, Serialize)]
struct SuccessResponse {
    status: String,
    data: Vec<EndpointConfig>,
}

#[get("/api/v1/endpoints")]
async fn get_all_endpoints() -> impl Responder {
    let config = ENDPOINT_CONFIG.lock().unwrap();
    let endpoints = config.iter().cloned().collect::<Vec<_>>();
    
    HttpResponse::Ok().json(SuccessResponse {
        status: "success".to_string(),
        data: endpoints,
    })
}
