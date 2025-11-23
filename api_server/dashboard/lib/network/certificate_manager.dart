import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

// 证书文件数据类
class CertificateFile {
  final String filename;
  final String description;

  CertificateFile(this.filename, this.description);
}

// 证书配置组件
class CertificateManager extends StatelessWidget {
  final String networkId;
  final String networkName;

  const CertificateManager({
    Key? key,
    required this.networkId,
    required this.networkName,
  }) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return SingleChildScrollView(
      padding: const EdgeInsets.all(20),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          const Text(
            'Certificate Configuration',
            style: TextStyle(fontSize: 20, fontWeight: FontWeight.bold),
          ),
          const SizedBox(height: 20),
          
          // CA证书配置
          _buildCertificateCard(
            title: 'CA Certificate',
            icon: Icons.security,
            files: [
              CertificateFile('ca-cert.pem', 'CA Root Certificate'),
              CertificateFile('ca-key.pem', 'CA Private Key'),
            ],
            context: context,
          ),
          
          const SizedBox(height: 20),
          
          // 服务器证书配置
          _buildCertificateCard(
            title: 'Server Certificate',
            icon: Icons.cloud,
            files: [
              CertificateFile('server-cert.pem', 'Server Certificate'),
              CertificateFile('server-key.pem', 'Server Private Key'),
            ],
            context: context,
          ),
          
          const SizedBox(height: 20),
          
          // 证书操作按钮
          _buildCertificateActions(),
        ],
      ),
    );
  }

  Widget _buildCertificateCard({
    required String title,
    required IconData icon,
    required List<CertificateFile> files,
    required BuildContext context,
  }) {
    return Card(
      elevation: 2,
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Row(
              children: [
                Icon(icon, color: Colors.blue),
                const SizedBox(width: 8),
                Text(
                  title,
                  style: const TextStyle(
                    fontSize: 18,
                    fontWeight: FontWeight.bold,
                  ),
                ),
              ],
            ),
            const SizedBox(height: 16),
            ...files.map((file) => _buildCertificateFileItem(file, context)),
          ],
        ),
      ),
    );
  }

  Widget _buildCertificateFileItem(CertificateFile file, BuildContext context) {
    return Container(
      margin: const EdgeInsets.only(bottom: 8),
      padding: const EdgeInsets.all(12),
      decoration: BoxDecoration(
        border: Border.all(color: Colors.grey.shade300),
        borderRadius: BorderRadius.circular(8),
      ),
      child: Row(
        children: [
          Expanded(
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(
                  file.filename,
                  style: const TextStyle(
                    fontWeight: FontWeight.bold,
                    fontSize: 14,
                  ),
                ),
                const SizedBox(height: 4),
                Text(
                  file.description,
                  style: TextStyle(
                    fontSize: 12,
                    color: Colors.grey.shade600,
                  ),
                ),
              ],
            ),
          ),
          IconButton(
            icon: const Icon(Icons.content_copy, size: 16),
            onPressed: () => _copyCertificateFile(file.filename, context),
            tooltip: 'Copy file path',
          ),
          IconButton(
            icon: const Icon(Icons.download, size: 16),
            onPressed: () => _downloadCertificateFile(file.filename, context),
            tooltip: 'Download file',
          ),
        ],
      ),
    );
  }

  Widget _buildCertificateActions() {
    return Card(
      elevation: 2,
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            const Text(
              'Certificate Actions',
              style: TextStyle(
                fontSize: 18,
                fontWeight: FontWeight.bold,
              ),
            ),
            const SizedBox(height: 16),
            Row(
              children: [
                ElevatedButton.icon(
                  onPressed: _generateNewCertificates,
                  icon: const Icon(Icons.add_circle_outline),
                  label: const Text('Generate New Certificates'),
                ),
                const SizedBox(width: 16),
                ElevatedButton.icon(
                  onPressed: _uploadCertificates,
                  icon: const Icon(Icons.upload_file),
                  label: const Text('Upload Certificates'),
                ),
              ],
            ),
          ],
        ),
      ),
    );
  }

  void _copyCertificateFile(String filename, BuildContext context) {
    Clipboard.setData(ClipboardData(text: filename));
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(
        content: Text('$filename path copied to clipboard'),
        duration: const Duration(seconds: 2),
      ),
    );
  }

  void _downloadCertificateFile(String filename, BuildContext context) {
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(
        content: Text('Downloading $filename...'),
        duration: const Duration(seconds: 2),
      ),
    );
  }

  void _generateNewCertificates() {
    print('Generating new certificates for network: $networkName ($networkId)');
  }

  void _uploadCertificates() {
    print('Uploading certificates for network: $networkName ($networkId)');
  }
}