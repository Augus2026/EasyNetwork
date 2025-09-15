import 'dart:convert';
import 'package:flutter/cupertino.dart';
import 'package:flutter/foundation.dart';
import 'package:http/http.dart' as http;
import 'network_info.dart';

String baseUrl = 'http://localhost:1000';

// 获取网络列表
Future<List<NetworkInfo>> getNetworks() async {
  final response = await http.get(
    Uri.parse('$baseUrl/api/v1/networks')
  );
  if (response.statusCode == 200) {
    final result = jsonDecode(response.body) as Map<String, dynamic>;
    debugPrint('get networks: $result');
    if(result['status'] == 'success') {
      return (result['data'] as List)
        .map((e) => NetworkInfo.fromJson(e))
        .toList();
    } else {
      throw Exception('Failed to load networks: ${result['message']}');
    }
  } else {
    throw Exception('Failed to load networks');
  }
}

// 添加网络
Future<NetworkInfo> addNetwork(BasicInfo network) async {
  final response = await http.post(
    Uri.parse('$baseUrl/api/v1/networks'),
    headers: {'Content-Type': 'application/json'},
    body: jsonEncode(network.toJson()),
  );
  if (response.statusCode == 200) {
    final result = jsonDecode(response.body) as Map<String, dynamic>;
    debugPrint('add network: $result');
    if (result['status'] == 'success') {
      return NetworkInfo.fromJson(result['data']);
    } else {
      throw Exception('Failed to add network: ${result['message']}');
    }
  } else {
    throw Exception('Failed to add network');
  }
}

// 删除网络
Future<bool> delNetwork(String networkId) async {
  final response = await http.delete(
    Uri.parse('$baseUrl/api/v1/networks/$networkId'),
  );
  if(response.statusCode == 200) {
    final result = jsonDecode(response.body) as Map<String, dynamic>;
    debugPrint('del network: $result');
    if (result['status'] == 'success') {
      return true;
    } else {
      return false;
    }
  } else {
    throw Exception('Failed to del network');
  }
}

// 更新网络
Future<bool> updateNetwork(NetworkInfo network) async {
  final response = await http.put(
    Uri.parse('$baseUrl/api/v1/networks/${network.basic.id}'),
    headers: {'Content-Type': 'application/json'},
    body: jsonEncode(network.toJson()),
  );
  if (response.statusCode == 200) {
    final result = jsonDecode(response.body) as Map<String, dynamic>;
    debugPrint('update network: $result');
    if (result['status'] == 'success') {
      return true;
    } else {
      return false;
    }
  } else {
    throw Exception('Failed to update network');
  }
}

// 获取网络成员
Future<List<Member>> getNetworkMembers(String networkId) async {
  final response = await http.get(
    Uri.parse('$baseUrl/api/v1/networks/$networkId/members'),
  );
  if (response.statusCode == 200) {
    final result = jsonDecode(response.body) as Map<String, dynamic>;
    debugPrint('get network members: $result');
    if (result['status'] == 'success') {
      return (result['data'] as List)
        .map((e) => Member.fromJson(e))
        .toList();
    } else {
      throw Exception('Failed to load network members: ${result['message']}');
    }
  } else {
    throw Exception('Failed to load network members');
  }
}