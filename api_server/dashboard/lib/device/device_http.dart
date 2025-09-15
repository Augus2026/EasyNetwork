import 'dart:convert';
import 'package:flutter/cupertino.dart';
import 'package:flutter/foundation.dart';
import 'package:http/http.dart' as http;
import 'device_info.dart';

String baseUrl = 'http://localhost:1000';

// 获取设备列表
Future<List<DeviceInfo>> getDevices() async {
  final response = await http.get(
    Uri.parse('$baseUrl/api/v1/devices')
  );
  if (response.statusCode == 200) {
    final result = jsonDecode(response.body) as Map<String, dynamic>;
    debugPrint('get devices: $result');
    if(result['status'] == 'success') {
      return (result['data'] as List)
        .map((e) => DeviceInfo.fromJson(e))
        .toList();
    } else {
      throw Exception('Failed to load devices: ${result['message']}');
    }
  } else {
    throw Exception('Failed to load devices');
  }
}
