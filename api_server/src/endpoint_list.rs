use actix_web::{get, HttpResponse, Responder};
use serde::Serialize;
use crate::config::{get_all_endpoints, EndpointStatus};

#[derive(Debug, Serialize)]
struct SuccessResponse {
    status: String,
    data: Vec<EndpointStatus>,
}

#[get("/api/v1/endpoints")]
async fn get_all_endpoints_handler() -> impl Responder {
    let endpoints = get_all_endpoints();
    
    HttpResponse::Ok().json(SuccessResponse {
        status: "success".to_string(),
        data: endpoints,
    })
}
