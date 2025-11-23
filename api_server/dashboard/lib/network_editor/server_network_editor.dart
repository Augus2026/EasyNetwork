import 'package:flutter/material.dart';
import '../network/network_info.dart';

// 服务器地址设置
class ServerNetworkEditor extends StatefulWidget {
  final NetworkInfo network;

  const ServerNetworkEditor({super.key, required this.network});

  @override
  State<ServerNetworkEditor> createState() => _ServerNetworkEditorState();
}

class _ServerNetworkEditorState extends State<ServerNetworkEditor> {
  final replyAddressController = TextEditingController();
  final replyPortController = TextEditingController();

  @override
  void initState() {
    super.initState();

    replyAddressController.addListener(() {
      widget.network.server.replyAddress = replyAddressController.text;
    });

    replyPortController.addListener(() {
      widget.network.server.replyPort = replyPortController.text;
    });
  }

  @override
  void dispose() {
    replyAddressController.dispose();
    replyPortController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Card(
      margin: const EdgeInsets.only(bottom: 20),
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            TextField(
              controller: replyAddressController,
              decoration: InputDecoration(
                labelText: 'Reply Address',
                hintText: widget.network.server.replyAddress,
                floatingLabelBehavior: FloatingLabelBehavior.always,
              ),
            ),
            TextField(
              controller: replyPortController,
              decoration: InputDecoration(
                labelText: 'Reply Port',
                hintText: widget.network.server.replyPort,
                floatingLabelBehavior: FloatingLabelBehavior.always,
              ),
            ),
          ],
        ),
      ),
    );
  }
}
