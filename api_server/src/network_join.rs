use actix_web::{post, web, HttpResponse, Responder};
use chrono::Utc;
use serde::{Deserialize, Serialize};
use validator::Validate;
use std::sync::atomic::{AtomicU8, Ordering};
use std::collections::HashMap;
use crate::config::{InterfaceInfo, ServerInfo, NetworkMemberStatus, NETWORK_STATUS};

// 在文件顶部添加原子计数器，初始值为100
static IP_COUNTER: AtomicU8 = AtomicU8::new(100);

#[derive(Debug, Deserialize, Validate)]
struct JoinRequest {
    #[validate(length(min = 1, message = "device_id cannot be empty"))]
    device_id: String,
    
    #[validate(length(min = 1, message = "network_name cannot be empty"))]
    network_name: String,
}

#[derive(Debug, Serialize)]
struct SuccessResponse {
    status: String,
    data: JoinResponseData,
}

#[derive(Debug, Serialize)]
struct JoinResponseData {
    interface: InterfaceInfo,
    server: ServerInfo,
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

#[post("/api/v1/networks/join")]
async fn join_network(body: web::Json<JoinRequest>) -> impl Responder {
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

    // 获取并递增IP地址的最后一位
    let ip_last_octet = IP_COUNTER.fetch_add(1, Ordering::SeqCst);
    // 当计数器超过200时重置为100
    let ip_last_octet = if ip_last_octet > 200 { 100 } else { ip_last_octet };
    let ip_address = format!("10.10.10.{}", ip_last_octet);

    // 模拟返回数据
    let response_data = JoinResponseData {
        interface: InterfaceInfo {
            name: body.network_name.clone(),
            desc: format!("{} network", body.network_name),
            ipv4_address: ip_address,
            subnet_mask: "24".to_string(),
            mtu: 1400,
            domain: "sub1".to_string(),
            name_server: "114.114.114.114,8.8.8.8".to_string(),
            search_list: "EasyNetwork.local".to_string(),
        },
        server: ServerInfo {
            reply_address: "cn-easy-network.com".to_string(),
            reply_port: "1234".to_string(),
            expires_at: Utc::now() + chrono::Duration::hours(24),
            expires: "24h".to_string(),
        },
    };

    // 更新NETWORK_STATUS
    let mut network_status = NETWORK_STATUS.lock().unwrap();
    let network = network_status.entry(body.network_name.clone())
        .or_insert_with(|| HashMap::new());
    
    network.insert(body.device_id.clone(), NetworkMemberStatus {
        last_updated: chrono::Utc::now(),
        interface: response_data.interface.clone(),
        server: response_data.server.clone()
    });

    HttpResponse::Ok().json(SuccessResponse {
        status: "success".to_string(),
        data: response_data,
    })
}
