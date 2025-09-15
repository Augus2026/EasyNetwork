import 'package:flutter/material.dart';
import 'package:net/network/network_info.dart';
import 'package:net/network/network_http.dart';

class NetworkMember extends StatefulWidget {
  final String networkId;
  
  const NetworkMember({super.key, required this.networkId});

  @override
  State<NetworkMember> createState() => _NetworkMemberState();
}

class _NetworkMemberState extends State<NetworkMember> {
  List<Member> _members = [];

  @override
  bool initState() {
    super.initState();
    
    getNetworkMembers(widget.networkId).then((value) {
      setState(() {
        _members = value;
      });
    }).catchError((error) {
      debugPrint('Error fetching network members: $error');
    });

    return true;
  }
  
  @override
  Widget build(BuildContext context) {
    return LayoutBuilder(
      builder: (context, constraints) {
        return SingleChildScrollView(
          scrollDirection: Axis.horizontal,
          child: ConstrainedBox(
            constraints: BoxConstraints(minWidth: constraints.maxWidth),
            child: DataTable(
              columnSpacing: 12,
              columns: const [
                DataColumn(label: Text('Edit')),
                DataColumn(label: Text('Auth')),
                DataColumn(label: Text('Address')),
                DataColumn(label: Text('Name/Desc')),
                DataColumn(label: Text('Managed IPs')),
                DataColumn(label: Text('mtu')),
                DataColumn(label: Text('Last Seen')),
                DataColumn(label: Text('Version')),
                DataColumn(label: Text('Physical IP')),
              ],
              rows: _members.map((member) => DataRow(
                cells: [
                  DataCell(
                    IconButton(
                      icon: const Icon(Icons.edit),
                      onPressed: () => _showEditMemberDialog(context, member),
                    ),
                  ),
                  DataCell(Text(member.auth == "true" ? '✓' : '✗')),
                  DataCell(Text(member.macAddress)),
                  DataCell(Text('${member.name}\n${member.desc}')),
                  DataCell(Text('${member.ipAddress}\n${member.subnetMask}')),
                  DataCell(Text(member.mtu.toString())),
                  DataCell(Text(member.lastSeen)),
                  DataCell(Text(member.version)),
                  DataCell(Text(member.physicalIP)),
                ],
              )).toList(),
            ),
          ),
        );
      },
    );
  }

  void _showEditMemberDialog(BuildContext context, Member member) {
    final authController = TextEditingController(text: member.auth);
    final addressController = TextEditingController(text: member.macAddress);
    final nameController = TextEditingController(text: member.name);
    final descController = TextEditingController(text: member.desc);
    final ipAddressController = TextEditingController(text: member.ipAddress);
    final subnetMaskController = TextEditingController(text: member.subnetMask);
    final mtuController = TextEditingController(text: member.mtu.toString());

    showDialog(
      context: context,
      builder: (context) {
        return AlertDialog(
          title: const Text('Edit Member'),
          content: SizedBox(
            width: 500,
            height: 400,
            child: Column(
              mainAxisSize: MainAxisSize.min,
              children: [
                TextField(
                  controller: authController,
                  decoration: const InputDecoration(labelText: 'Auth'),
                ),
                TextField(
                  controller: addressController,
                  decoration: const InputDecoration(labelText: 'Address'),
                ),
                TextField(
                  controller: nameController,
                  decoration: const InputDecoration(labelText: 'Name'),
                ),
                TextField(
                  controller: descController,
                  decoration: const InputDecoration(labelText: 'Description'),
                ),
                TextField(
                  controller: ipAddressController,
                  decoration: const InputDecoration(labelText: 'IP Address'),
                ),
                TextField(
                  controller: subnetMaskController,
                  decoration: const InputDecoration(labelText: 'Subnet Mask'),
                ),
                TextField(
                  controller: mtuController,
                  decoration: const InputDecoration(labelText: 'mtu'),
                ),
              ],
            ),
          ),
          actions: [
            TextButton(
              onPressed: () => Navigator.pop(context),
              child: const Text('Cancel'),
            ),
            TextButton(
              onPressed: () {
                setState(() {
                  // 更新成员数据
                  member.auth = authController.text;
                  member.macAddress = addressController.text;
                  member.name = nameController.text;
                  member.desc = descController.text;
                  member.ipAddress = ipAddressController.text;
                  member.subnetMask = subnetMaskController.text;
                  member.mtu = int.parse(mtuController.text);
                });
                Navigator.pop(context);
              },
              child: const Text('Save'),
            ),
          ],
        );
      },
    ).then((_) {
      // 对话框关闭后释放控制器
      authController.dispose();
      addressController.dispose();
      nameController.dispose();
      descController.dispose();
      ipAddressController.dispose();
      subnetMaskController.dispose();
      mtuController.dispose();
    });
  }
}