import 'package:flutter/material.dart';
import '../network/network_info.dart';

// dns设置
class DnsNetworkEditor extends StatefulWidget {
  final NetworkInfo network;

  const DnsNetworkEditor({super.key, required this.network});

  @override
  State<DnsNetworkEditor> createState() => _DnsNetworkEditorState();
}

class _DnsNetworkEditorState extends State<DnsNetworkEditor> {
  final domainController = TextEditingController();
  final nameServerController = TextEditingController();
  final searchListController = TextEditingController();

  @override
  void initState() {
    super.initState();

    domainController.addListener(() {
      widget.network.dns.domain = domainController.text;
    });

    nameServerController.addListener(() {
      widget.network.dns.nameServer = nameServerController.text;
    });

    searchListController.addListener(() {
      widget.network.dns.searchList = searchListController.text;
    });
  }

  @override
  void dispose() {
    domainController.dispose();
    nameServerController.dispose();
    searchListController.dispose();
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
            const Text(
              'DNS',
              style: TextStyle(fontSize: 16, fontWeight: FontWeight.bold),
            ),
            TextField(
              controller: domainController,
              decoration: InputDecoration(
                labelText: 'Domain',
                hintText: widget.network.dns.domain,
                floatingLabelBehavior: FloatingLabelBehavior.always,
              ),
            ),
            TextField(
              controller: nameServerController,
              decoration: InputDecoration(
                labelText: 'Name Server',
                hintText: widget.network.dns.nameServer,
                floatingLabelBehavior: FloatingLabelBehavior.always,
              ),
            ),
            TextField(
              controller: searchListController,
              decoration: InputDecoration(
                labelText: 'Search List',
                hintText: widget.network.dns.searchList,
                floatingLabelBehavior: FloatingLabelBehavior.always,
              ),
            ),
          ],
        ),
      ),
    );
  }
}
