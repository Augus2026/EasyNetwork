import 'package:flutter/material.dart';

import '../network/network_info.dart';
import 'basic_network_editor.dart';
import 'route_network_editor.dart';
import 'dhcp_network_editor.dart';
import 'dns_network_editor.dart';
import 'server_network_editor.dart';

// 网络设置
class NetworkEditor extends StatefulWidget {
  final NetworkInfo network;

  const NetworkEditor({super.key, required this.network});

  @override
  State<NetworkEditor> createState() => _NetworkEditorState();
}

class _NetworkEditorState extends State<NetworkEditor> {

  @override
  Widget build(BuildContext context) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        const Text(
          'Basics',
          style: TextStyle(fontSize: 20, fontWeight: FontWeight.bold),
        ),
        BasicNetworkEditor(network: widget.network),
        const Text(
          'Advanced',
          style: TextStyle(fontSize: 20, fontWeight: FontWeight.bold),
        ),
        ManagedRoutesNetworkEditor(network: widget.network),
        Ipv4AutoAssignNetworkEditor(network: widget.network),
        DnsNetworkEditor(network: widget.network),
        ServerNetworkEditor(network: widget.network),
      ],
    );
  }
}