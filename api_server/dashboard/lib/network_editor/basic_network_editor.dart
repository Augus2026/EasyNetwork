import 'package:flutter/material.dart';
import '../network/network_info.dart';

// 基础设置
class BasicNetworkEditor extends StatefulWidget {
  final NetworkInfo network;

  const BasicNetworkEditor({super.key, required this.network});

  @override
  State<BasicNetworkEditor> createState() => _BasicNetworkEditorState();
}

class _BasicNetworkEditorState extends State<BasicNetworkEditor> {
  final idController = TextEditingController();
  final nameController = TextEditingController();
  final descController = TextEditingController();

  @override
  void initState() {
    super.initState();

    idController.addListener(() {
      widget.network.basic.id = idController.text;
    });
    nameController.addListener(() {
      widget.network.basic.name = nameController.text;
    });
    descController.addListener(() {
      widget.network.basic.desc = descController.text;
    });
  }

  @override
  void dispose() {
    idController.dispose();
    nameController.dispose();
    descController.dispose();
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
              controller: idController,
              decoration: InputDecoration(
                labelText: 'Network ID',
                hintText: widget.network.basic.id,
                floatingLabelBehavior: FloatingLabelBehavior.always,
              ),
            ),
            TextField(
              controller: nameController,
              decoration: InputDecoration(
                labelText: 'Name',
                hintText: widget.network.basic.name,
                floatingLabelBehavior: FloatingLabelBehavior.always,
              ),
            ),
            TextField(
              controller: descController,
              decoration: InputDecoration(
                labelText: 'Description',
                hintText: widget.network.basic.desc,
                floatingLabelBehavior: FloatingLabelBehavior.always,
              ),
            ),
            const SizedBox(height: 10),
            const Text(
              'Access Control',
              style: TextStyle(fontSize: 16, fontWeight: FontWeight.bold),
            ),
            const SizedBox(height: 10),
            RadioListTile(
              title: const Text('Private'),
              subtitle: const Text(
                'Devices must be authorized to become members',
              ),
              value: true,
              groupValue: widget.network.basic.isPrivate,
              onChanged: (value) {
                if (value != null) {
                  setState(() {
                    widget.network.basic.isPrivate = value;
                  });
                }
              },
            ),
            RadioListTile(
              title: const Text('Public'),
              subtitle: const Text(
                'Any node that knows the Network ID can become a member. Members cannot be de-authorized or deleted. Members that haven\'t been online in 30 days will be removed, but can rejoin.',
              ),
              value: false,
              groupValue: widget.network.basic.isPrivate,
              onChanged: (value) {
                if (value != null) {
                  setState(() {
                    widget.network.basic.isPrivate = value;
                  });
                }
              },
            ),
          ],
        ),
      ),
    );
  }
}
