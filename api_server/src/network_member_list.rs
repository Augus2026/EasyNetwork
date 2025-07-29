use std::collections::HashMap;
use actix_web::{get, web, HttpResponse, Responder};
use serde::Serialize;
use crate::config::{get_all_network_members, NetworkMemberStatus};

#[derive(Debug, Serialize)]
struct SuccessResponse {
    status: String,
    data: HashMap<String, NetworkMemberStatus>,
}

#[derive(Debug, Serialize)]
struct ErrorResponse {
    status: String,
    error: String,
}

#[get("/api/v2/networks/{network_name}/members")]
async fn get_all_network_members_handler(path: web::Path<String>) -> impl Responder {
    let network_name = path.into_inner();
    
    match get_all_network_members(&network_name) {
        Some(members) => HttpResponse::Ok().json(SuccessResponse {
            status: "success".to_string(),
            data: members,
        }),
        None => HttpResponse::NotFound().json(ErrorResponse {
            status: "error".to_string(),
            error: "Network not found".to_string(),
        })
    }
}
