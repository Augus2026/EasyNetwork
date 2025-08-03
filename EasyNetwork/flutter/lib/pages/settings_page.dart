import 'package:flutter/material.dart';
import 'package:shared_preferences/shared_preferences.dart';

class SettingsPage extends StatefulWidget {
  @override
  _SettingsPageState createState() => _SettingsPageState();
}

class _SettingsPageState extends State<SettingsPage> {
  final _formKey = GlobalKey<FormState>();
  final _apiAddressController = TextEditingController();
  final _apiPortController = TextEditingController();
  final _replyAddressController = TextEditingController();
  final _replyPortController = TextEditingController();
  bool _forceUseApiServer = false;
  bool _forceUseReplyServer = false;

  @override
  void initState() {
    super.initState();
    _loadLocalSettings();
  }

  @override
  void dispose() {
    _apiAddressController.dispose();
    _apiPortController.dispose();
    _replyAddressController.dispose();
    _replyPortController.dispose();
    super.dispose();
  }

  Future<void> _loadLocalSettings() async {
    SharedPreferences prefs = await SharedPreferences.getInstance();
    setState(() {
      _forceUseApiServer = prefs.getBool('forceUseApiServer') ?? false;
      _apiAddressController.text = prefs.getString('apiServerAddress') ?? '';
      _apiPortController.text = prefs.getInt('apiServerPort')?.toString() ?? '';

      _forceUseReplyServer = prefs.getBool('forceUseReplyServer') ?? false;
      _replyAddressController.text = prefs.getString('replyServerAddress') ?? '';
      _replyPortController.text = prefs.getInt('replyServerPort')?.toString() ?? '';
    });
  }

  Future<void> _saveLocalSettings() async {
    SharedPreferences prefs = await SharedPreferences.getInstance();
    prefs.setBool('forceUseApiServer', _forceUseApiServer);
    prefs.setString('apiServerAddress', _apiAddressController.text);
    prefs.setInt('apiServerPort', int.parse(_apiPortController.text));
    
    prefs.setBool('forceUseReplyServer', _forceUseReplyServer);
    prefs.setString('replyServerAddress', _replyAddressController.text);
    prefs.setInt('replyServerPort', int.parse(_replyPortController.text));
  }

  Future<void> _saveSettings() async {
    if (_formKey.currentState!.validate()) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text('设置已保存')),
      );
      await _saveLocalSettings();
    }
  }

  Widget _buildApiServerSettings() {
    return Column(
      children: [
        Text('api服务器设置', style: TextStyle(fontWeight: FontWeight.bold)),
        SwitchListTile(
          title: Text('强制使用api服务器'),
          value: _forceUseApiServer,
          onChanged: (bool value) {
            setState(() {
              _forceUseApiServer = value;
            });
          },
        ),
        TextFormField(
          controller: _apiAddressController,
          decoration: InputDecoration(labelText: 'api服务器IP'),
          validator: (value) => _validateIp(value),
        ),
        TextFormField(
          controller: _apiPortController,
          decoration: InputDecoration(labelText: 'api端口'),
          validator: (value) => _validatePort(value),
        ),
      ],
    );
  }

  Widget _buildReplyServerSettings() {
    return Column(
      children: [
        Text('reply服务器设置', style: TextStyle(fontWeight: FontWeight.bold)),
        SwitchListTile(
          title: Text('强制使用reply服务器'),
          value: _forceUseReplyServer,
          onChanged: (bool value) {
            setState(() {
              _forceUseReplyServer = value;
            });
          },
        ),
        TextFormField(
          controller: _replyAddressController,
          decoration: InputDecoration(labelText: 'reply服务器IP'),
          validator: (value) => _validateIp(value),
        ),
        TextFormField(
          controller: _replyPortController,
          decoration: InputDecoration(labelText: 'reply端口'),
          validator: (value) => _validatePort(value),
        ),
      ],
    );
  }

  Widget _buildSaveSettings() {
    return ElevatedButton(
      onPressed: () async {
        await _saveSettings();
        Navigator.pop(context);
      },
      child: Text('保存设置'),
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: Text('设置')),
      body: Padding(
        padding: EdgeInsets.all(16.0),
        child: Form(
          key: _formKey,
          child: ListView(
            children: [
              _buildApiServerSettings(),
              SizedBox(height: 20),
              _buildReplyServerSettings(),
              SizedBox(height: 20),
              _buildSaveSettings(),
            ],
          ),
        ),
      ),
    );
  }
}

String? _validateIp(String? value) {
  if (value == null || value.isEmpty) return '请输入IP地址';
  final ipRegex = RegExp(r'^((25[0-5]|2[0-4]\d|[01]?\d\d?)\.){3}(25[0-5]|2[0-4]\d|[01]?\d\d?)$');
  if (!ipRegex.hasMatch(value)) return '请输入有效的IP地址';
  return null;
}

String? _validatePort(String? value) {
  if (value == null || value.isEmpty) return '请输入端口号';
  final port = int.tryParse(value);
  if (port == null || port < 1 || port > 65535) return '请输入有效的端口号 (1-65535)';
  return null;
}