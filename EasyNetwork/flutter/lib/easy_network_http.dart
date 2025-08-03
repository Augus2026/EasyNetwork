import 'dart:convert';
import 'dart:io';
import 'package:flutter/material.dart';
import 'package:http/http.dart' as http;
import 'package:device_info_plus/device_info_plus.dart';
import 'package:system_info/system_info.dart';
import 'config.dart';

Future<Map<String, dynamic>> _getDeviceInfo() async {
  final deviceInfo = DeviceInfoPlugin();
  if (Platform.isWindows) {
    final windowsInfo = await deviceInfo.windowsInfo;
    return {
      // 设备规格
      'computer_name': windowsInfo.computerName,
      'num_processor': windowsInfo.numberOfCores.toString(),
      'memory': windowsInfo.systemMemoryInMegabytes.toString(),
      'product_id': windowsInfo.productId,
      'device_id': windowsInfo.deviceId,
      // windwos 规格
      'user_name': windowsInfo.userName,
      'product_name': windowsInfo.productName,
      'edition_id': windowsInfo.editionId,
      'display_version': windowsInfo.displayVersion,
      'install_date': windowsInfo.installDate.toIso8601String(),
      'build_number': windowsInfo.buildNumber.toString(),
    };
  }
  return {
    // 设备规格
    'computer_name': '',
    'num_processor': '',
    'memory': '',
    'product_id': '',
    'device_id': '',
    // windwos 规格
    'user_name': '',
    'product_name': '',
    'edition_id': '',
    'display_version': '',
    'install_date': '',
    'build_number': '',
  };
}

// 获取在线状态
Future<bool> report_sysinfo({
  required String uuid,
}) async {
  try {
    String domain = await get_api_server_address();
    final url = Uri.parse('http://$domain/api/v1/sysinfo');
    final headers = {
      'Accept': 'application/json',
      'Content-Type': 'application/json',
    };

    final body = jsonEncode({
      'uuid': uuid,
      'deviceInfo': await _getDeviceInfo(),
    });

    final response = await http.post(
      url,
      headers: headers,
      body: body,
    );

    final responseData = jsonDecode(response.body) as Map<String, dynamic>;
    if (response.statusCode == 200) {
      return responseData['status'] == 'success';
    } else if (response.statusCode == 400) {
      return false;
    } else {
      throw Exception('Failed to report system info: ${response.statusCode}');
    }
  } catch (e) {
    throw Exception('Network error: $e');
  }
}

// 更新在线状态
Future<bool> update_online_status({
  required String uuid,
}) async {
  try {
    String domain = await get_api_server_address();
    final url = Uri.parse('http://$domain/api/v1/online_status');
    final headers = {
      'Accept': 'application/json',
      'Content-Type': 'application/json',
    };
    final body = jsonEncode({
      'uuid': uuid,
    });

    final response = await http.post(
      url,
      headers: headers,
      body: body,
    ).timeout(const Duration(seconds: 3));

    final responseData = jsonDecode(response.body) as Map<String, dynamic>;
    if (response.statusCode == 200) {
      return responseData['status'] == 'success';
    } else if (response.statusCode == 400) {
      final error = responseData['error'] as Map<String, dynamic>?;
      throw Exception(error?['message'] ?? 'Invalid request');
    } else {
      throw Exception('Failed to update online status: ${response.statusCode}');
    }
  } catch (e) {
    print('update_online_status error: $e');
    throw Exception('Network error: $e');
  }
}

// 更新成员在线状态
Future<bool> update_member_online_status({
  required String deviceId,
  required String networkName
}) async {
  try {
    String domain = await get_api_server_address();
    final url = Uri.parse('http://$domain/api/v1/member_online_status');
    final headers = {
      'Accept': 'application/json',
      'Content-Type': 'application/json',
    };
    final body = jsonEncode({
      'device_id': deviceId,
      'network_name': networkName,
    });

    final response = await http.post(
      url,
      headers: headers,
      body: body,
    );

    final responseData = jsonDecode(response.body) as Map<String, dynamic>;
    if (response.statusCode == 200) {
      return responseData['online_status'] == 'online';
    } else if (response.statusCode == 400) {
      final error = responseData['error'] as Map<String, dynamic>?;
      throw Exception(error?['message'] ?? 'Invalid request');
    } else {
      throw Exception('Failed to update online status: ${response.statusCode}');
    }
  } catch (e) {
    throw Exception('Network error: $e');
  }
}


Future<List<Map<String, dynamic>>> get_network_members({
  required String networkId,
}) async {
  try {
    String domain = await get_api_server_address();
    final url = Uri.parse('http://$domain/api/v1/networks/$networkId/members');
    final headers = {
      'Accept': 'application/json',
      'Content-Type': 'application/json',
    };

    final response = await http.get(
      url,
      headers: headers,
    );

    final responseData = jsonDecode(response.body) as Map<String, dynamic>;
    if (response.statusCode == 200) {
      final members = (responseData['data'] as List)
          .map((member) => {
                'name': member['member_id'],
                'status': member['online'],
                'icon': member['online'] ? Icons.person : Icons.person_outline,
              })
          .toList();
      return members;
    } else if (response.statusCode == 400) {
      throw Exception('获取成员失败: 无效请求');
    } else {
      throw Exception('获取成员失败: ${response.statusCode}');
    }
  } catch (e) {
    throw Exception('网络错误: $e');
  }
}

Future<Map<String, dynamic>> joinNetwork({
  required String deviceId,
  required String networkName,
}) async {
  try {
    String domain = await get_api_server_address();
    final url = Uri.parse('http://$domain/api/v1/networks/join');
    final headers = {
      'Accept': 'application/json',
      'Content-Type': 'application/json',
    };
    final body = jsonEncode({
      'device_id': deviceId,
      'network_name': networkName,
    });

    final response = await http.post(
      url,
      headers: headers,
      body: body,
    );

    final responseData = jsonDecode(response.body) as Map<String, dynamic>;
    if (response.statusCode == 200) {
      return responseData['data'] as Map<String, dynamic>;
    } else if (response.statusCode == 400) {
      final error = responseData['error'] as Map<String, dynamic>?;
      throw Exception(error?['message'] ?? 'Invalid request');
    } else {
      throw Exception('Failed to join network: ${response.statusCode}');
    }
  } catch (e) {
    throw Exception('Network error: $e');
  }
}

Future<void> leaveNetwork({
  required String deviceId,
  required String networkName,
}) async {
  try {
    String domain = await get_api_server_address();
    final url = Uri.parse('http://$domain/api/v1/networks/leave');
    final headers = {
      'Accept': 'application/json',
      'Content-Type': 'application/json',
    };
    final body = jsonEncode({
      'device_id': deviceId,
      'network_name': networkName,
    });

    final response = await http.post(
      url,
      headers: headers,
      body: body,
    );

    if (response.statusCode == 200) {
      return;
    } else if (response.statusCode == 400) {
      final error = jsonDecode(response.body)['error'] as Map<String, dynamic>?;
      throw Exception(error?['message'] ?? 'Invalid request');
    } else {
      throw Exception('Failed to leave network: ${response.statusCode}');
    }
  } catch (e) {
    throw Exception('Network error: $e');
  }
}

Future<List<Map<String, dynamic>>> update_route({
  required String deviceId,
  required String networkName,
}) async {
  try {
    String domain = await get_api_server_address();
    final url = Uri.parse('http://$domain/api/v1/route_list');
    final headers = {
      'Accept': 'application/json',
      'Content-Type': 'application/json',
    };
    final body = jsonEncode({
      'device_id': deviceId,
      'network_name': networkName,
    });

    final response = await http.post(
      url,
      headers: headers,
      body: body,
    );

    final responseData = jsonDecode(response.body) as Map<String, dynamic>;
    if (response.statusCode == 200) {
      return (responseData['data'] as List).cast<Map<String, dynamic>>();
    } else if (response.statusCode == 400) {
      final error = responseData['error'] as Map<String, dynamic>?;
      throw Exception(error?['message'] ?? 'Invalid request');
    } else {
      throw Exception('Failed to update route: ${response.statusCode}');
    }
  } catch (e) {
    throw Exception('Network error: $e');
  }
}
