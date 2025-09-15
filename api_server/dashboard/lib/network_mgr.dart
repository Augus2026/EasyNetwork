import 'package:flutter/material.dart';
import 'dart:math';
import 'network/network_detail.dart';
import 'network/network_info.dart';
import 'network/network_http.dart';

import 'device/device_detail.dart';
import 'device/device_info.dart';
import 'device/device_http.dart';

class NetworkMgr extends StatefulWidget {
  const NetworkMgr({super.key});

  @override
  State<NetworkMgr> createState() => _NetworkMgrState();
}

class _NetworkMgrState extends State<NetworkMgr> {
  final List<NetworkInfo> networks = [];
  final int maxNetworks = 3;
  final int maxDevices = 10;

  int get totalNetworks {
    return networks.length;
  }

  int get totalDevices {
    return networks.fold(0, (sum, network) => sum + int.parse(network.basic.devices));
  }

  @override
  void initState() {
    super.initState();
    _loadNetworks();
  }

  @override
  void dispose() {
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: Row(
        children: [
          _buildNetworkLeftSection(),
          _buildNetworkRightSection(),
        ],
      ),
    );
  }

  Future<void> _loadNetworks() async {
    try {
      final loadedNetworks = await getNetworks();
      setState(() {
        networks.addAll(loadedNetworks);
      });
    } catch (e) {
      print('Error loading networks: $e');
    }
  }

  String _buildNetworkId() {
    final random = Random();
    const chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789';
    final id = String.fromCharCodes(Iterable.generate(6, (_) => chars.codeUnitAt(random.nextInt(chars.length))));
    return id;
  }

  void _showAddNetworkDialog(BuildContext context) {
    final id = _buildNetworkId();
    final devices = '0';
    final now = DateTime.now();
    final created = '${now.year}-${now.month.toString().padLeft(2, '0')}-${now.day.toString().padLeft(2, '0')}';
    
    TextEditingController idController = TextEditingController(text: id);
    TextEditingController nameController = TextEditingController();
    TextEditingController descController = TextEditingController();

    showDialog(
      context: context,
      builder: (context) {
        return AlertDialog(
          title: const Text('Create A Network'),
          content: SizedBox(
            width: 500,
            height: 400,
            child: Column(
              mainAxisSize: MainAxisSize.min,
              children: [
                TextField(
                  controller: idController,
                  decoration: const InputDecoration(
                    labelText: 'Network ID',
                    floatingLabelBehavior: FloatingLabelBehavior.always,
                  ),
                  readOnly: true,
                ),
                TextField(
                  controller: nameController,
                  decoration: const InputDecoration(
                    labelText: 'Name',
                    floatingLabelBehavior: FloatingLabelBehavior.always,
                  ),
                ),
                TextField(
                  controller: descController,
                  decoration: const InputDecoration(
                    labelText: 'Description',
                    floatingLabelBehavior: FloatingLabelBehavior.always,
                  ),
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
                BasicInfo basic = BasicInfo(
                  id: idController.text,
                  name: nameController.text,
                  desc: descController.text,
                  devices: devices,
                  created: created,
                  isPrivate: false,
                );

                addNetwork(basic).then((networkInfo) {
                  setState(() {
                    networks.add(networkInfo);
                  });
                }).catchError((error) {
                  print('Error adding network: $error');
                });

                Navigator.pop(context);
              },
              child: const Text('Add'),
            ),
          ],
        );
      },
    );
  }

  void _showNetworkDetail(NetworkInfo network) {
    Navigator.push(
      context,
      MaterialPageRoute(
        builder: (context) => NetworkDetail(
          network: network,
          onDeleteNetwork: () {
            delNetwork(network.basic.id).then((success) {
              if (success) {
                setState(() {
                  networks.removeWhere((n) => n.basic.id == network.basic.id);
                });
              }
            }).catchError((error) {
              debugPrint('Error deleting network: $error');
            });
            Navigator.pop(context);
          },
          onClose: () {
            updateNetwork(network).then((success) {
              debugPrint('update network success');
            }).catchError((error) {
              debugPrint('Error updating network: $error');
            });
            Navigator.pop(context);
          },
        ),
      ),
    );
  }

  Widget _buildNetworkSection() {
    return LayoutBuilder(
      builder: (context, constraints) {
        return SingleChildScrollView(
          scrollDirection: Axis.horizontal,
          child: ConstrainedBox(
            constraints: BoxConstraints(minWidth: constraints.maxWidth),
            child: DataTable(
              columnSpacing: 12,
              columns: const [
                DataColumn(label: Text('Network ID')),
                DataColumn(label: Text('Name')),
                DataColumn(label: Text('Description')),
                DataColumn(label: Text('devices')),
                DataColumn(label: Text('Created')),
              ],
              rows:
                networks.map((network) {
                  return DataRow(
                    cells: [
                      DataCell(Text(network.basic.id)),
                      DataCell(Text(network.basic.name)),
                      DataCell(Text(network.basic.desc)),
                      DataCell(Text(network.basic.devices)),
                      DataCell(Text(network.basic.created)),
                    ],
                    onSelectChanged: (_) => _showNetworkDetail(network),
                  );
                }).toList(),
            ),
          ),
        );
      },
    );
  }

  void _showDevices() {
    getDevices().then((deviceInfoList) {
      Navigator.push(
        context,
        MaterialPageRoute(
          builder: (context) => DeviceDetail(
            deviceInfoList: deviceInfoList,
          ),
        ),
      );
    }).catchError((error) {
      print('Error getting devices: $error');
    });
  }

  Widget _buildNetworkLeftSection() {
    return Expanded(
      flex: 1,
      child: Container(
        color: Colors.grey[200],
        padding: const EdgeInsets.only(left: 20, top: 50, right: 20, bottom: 50),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            const Text('Your Networks'),
            const SizedBox(height: 20),
            Text('Networks: $totalNetworks / $maxNetworks'),
            const SizedBox(height: 20),
            Text('Included Devices: $totalDevices / $maxDevices'),
            const SizedBox(height: 20),
            Text('Total Devices: ${networks.fold(0, (sum, network) => sum + int.parse(network.basic.devices))}'),
            const SizedBox(height: 20),
            ElevatedButton(
              onPressed: _showDevices,
              child: const Text('Show Devices'),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildNetworkRightSection() {
    return Expanded(
      flex: 3,
      child: Container(
        color: Colors.white,
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Row(
              children: [
                ElevatedButton(
                  onPressed: () => _showAddNetworkDialog(context),
                  style: ElevatedButton.styleFrom(
                    backgroundColor: const Color.fromRGBO(255, 204, 142, 1),
                  ),
                  child: const Text('Create A Network'),
                ),
              ],
            ),
            const SizedBox(height: 16),
            _buildNetworkSection(),
          ],
        ),
      ),
    );
  }

}