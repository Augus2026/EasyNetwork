import 'package:flutter/material.dart';
import '../network/network_info.dart';

// 路由设置
class ManagedRoutesNetworkEditor extends StatefulWidget {
  final NetworkInfo network;

  const ManagedRoutesNetworkEditor({super.key, required this.network});

  @override
  State<ManagedRoutesNetworkEditor> createState() =>
      _ManagedRoutesNetworkEditorState();
}

class _ManagedRoutesNetworkEditorState extends State<ManagedRoutesNetworkEditor> {
  final List<TextEditingController> destControllers = [];
  final List<TextEditingController> netmaskControllers = [];
  final List<TextEditingController> gatewayControllers = [];
  final List<TextEditingController> metricControllers = [];

  @override
  void initState() {
    super.initState();
    for (var route in widget.network.routes) {
      _addNewRoute2(route);
    }
  }

  @override
  void dispose() {
    for (var controller in destControllers) {
      controller.dispose();
    }
    for (var controller in netmaskControllers) {
      controller.dispose();
    }
    for (var controller in gatewayControllers) {
      controller.dispose();
    }
    for (var controller in metricControllers) {
      controller.dispose();
    }
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
            Row(
              mainAxisAlignment: MainAxisAlignment.spaceBetween,
              children: [
                const Text(
                  'Managed Routes',
                  style: TextStyle(fontSize: 16, fontWeight: FontWeight.bold),
                ),
                const SizedBox(width: 10),
                ElevatedButton(
                  style: ElevatedButton.styleFrom(
                    backgroundColor: const Color.fromRGBO(255, 204, 142, 1),
                  ),
                  onPressed: _addNewRoute,
                  child: const Text('Add New Route'),
                ),
              ],
            ),
            const SizedBox(height: 10),
            ConstrainedBox(
              constraints: BoxConstraints(
                minWidth: MediaQuery.of(context).size.width - 32,
              ),
              child: _buildRouteTable(),
            ),
          ],
        ),
      ),
    );
  }

  // 添加新路由
  void _addNewRoute() {
    setState(() {
      final newRoute = RouteInfo(
        dest: '',
        netmask: '',
        gateway: '',
        metric: 0,
      );
      widget.network.routes.add(newRoute);
      _addNewRoute2(newRoute);
    });
  }

  // 添加新路由
  void _addNewRoute2(RouteInfo newRoute) {     
    final destController = TextEditingController();
    final netmaskController = TextEditingController();
    final gatewayController = TextEditingController();
    final metricController = TextEditingController();

    destControllers.add(destController);
    netmaskControllers.add(netmaskController);
    gatewayControllers.add(gatewayController);
    metricControllers.add(metricController);

    destController.addListener(() {
      newRoute.dest = destController.text;
    });
    netmaskController.addListener(() {
      newRoute.netmask = netmaskController.text;
    });
    gatewayController.addListener(() {
      newRoute.gateway = gatewayController.text;
    });
    metricController.addListener(() {
      newRoute.metric = int.tryParse(metricController.text) ?? 0;
    });
  }

  // 删除路由
  void _deleteRoute(int index) {
    setState(() {
      if (index >= 0 && index < widget.network.routes.length) {
        widget.network.routes.removeAt(index);
        
        // 移除对应的控制器
        destControllers[index].dispose();
        netmaskControllers[index].dispose();
        gatewayControllers[index].dispose();
        metricControllers[index].dispose();
        
        destControllers.removeAt(index);
        netmaskControllers.removeAt(index);
        gatewayControllers.removeAt(index);
        metricControllers.removeAt(index);
      }
    });
  }

  List<DataRow> _buildRouteRow() {
    return widget.network.routes.asMap().entries.map((entry) {
      final index = entry.key;
      final route = entry.value;
      return DataRow(
        cells: [
          DataCell(
            TextField(
              controller: destControllers[index],
              decoration: InputDecoration(
                hintText: route.dest,
                border: InputBorder.none,
              ),
            ),
          ),
          DataCell(
            TextField(
              controller: netmaskControllers[index],
              decoration: InputDecoration(
                hintText: route.netmask,
                border: InputBorder.none,
              ),
            ),
          ),
          DataCell(
            TextField(
              controller: gatewayControllers[index],
              decoration: InputDecoration(
                hintText: route.gateway,
                border: InputBorder.none,
              ),
            ),
          ),
          DataCell(
            TextField(
              controller: metricControllers[index],
              decoration: InputDecoration(
                hintText: route.metric.toString(),
                border: InputBorder.none
              ),
            ),
          ),
          DataCell(
            IconButton(
              icon: const Icon(Icons.delete),
              onPressed: () => _deleteRoute(index),
            ),
          ),
        ],
      );
    }).toList();
  }

  Widget _buildRouteTable() {
    return DataTable(
          columnSpacing: 5,
          columns: const [
            DataColumn(label: Text('Dest')),
            DataColumn(label: Text('Netmask')),
            DataColumn(label: Text('Gateway')),
            DataColumn(label: Text('Metric')),
            DataColumn(label: Text('Action')),
          ],
          rows: _buildRouteRow(),
    );
  }
}