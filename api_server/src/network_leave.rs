use actix_web::{post, web, HttpResponse, Responder};
use serde::{Deserialize, Serialize};
use validator::Validate;

#[derive(Debug, Deserialize, Validate)]
struct LeaveRequest {
    #[validate(length(min = 1, message = "device_id cannot be empty"))]
    device_id: String,
    
    #[validate(length(min = 1, message = "network_name cannot be empty"))]
    network_name: String,
}

#[derive(Debug, Serialize)]
struct SuccessResponse {
    status: String,
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

#[post("/api/v1/networks/leave")]
async fn leave_network(body: web::Json<LeaveRequest>) -> impl Responder {
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

    // 这里应该是实际的业务逻辑，例如从网络中移除设备
    // 简化示例：直接返回成功

    HttpResponse::Ok().json(SuccessResponse {
        status: "success".to_string(),
    })
}
