use actix_web::{post, web, HttpResponse, Responder};
use serde::{Deserialize, Serialize};
use validator::Validate;

#[derive(Debug, Deserialize, Validate)]
struct RouteListRequest {
    #[validate(length(min = 1, message = "device_id cannot be empty"))]
    device_id: String,
    
    #[validate(length(min = 1, message = "network_name cannot be empty"))]
    network_name: String,
}

#[derive(Debug, Serialize)]
struct RouteInfo {
    destination: String,
    netmask: String,
    gateway: String,
    metric: String,
}

#[derive(Debug, Serialize)]
struct SuccessResponse {
    status: String,
    data: Vec<RouteInfo>,
}

#[derive(Debug, Serialize)]
struct ErrorResponse {
    status: String,
    error: ErrorDetail,
}

#[derive(Debug, Serialize)]
struct ErrorDetail {
    code: String,
    message: String,
}

#[post("/api/v1/route_list")]
async fn get_route_list(body: web::Json<RouteListRequest>) -> impl Responder {
    println!("get_route_list");
    // 验证请求参数
    if let Err(validation_errors) = body.validate() {
        return HttpResponse::BadRequest().json(ErrorResponse {
            status: "error".to_string(),
            error: ErrorDetail {
                code: "INVALID_REQUEST".to_string(),
                message: validation_errors.to_string(),
            },
        });
    }

    // 模拟返回路由列表数据
    let routes = vec![
        RouteInfo {
            destination: "10.10.20.0".to_string(),
            netmask: "255.255.255.0".to_string(),
            gateway: "10.10.20.1".to_string(),
            metric: "100".to_string(),
        },
        RouteInfo {
            destination: "10.10.30.0".to_string(),
            netmask: "255.255.255.0".to_string(),
            gateway: "10.10.30.1".to_string(),
            metric: "100".to_string(),
        },
    ];

    HttpResponse::Ok().json(SuccessResponse {
        status: "success".to_string(),
        data: routes,
    })
}
