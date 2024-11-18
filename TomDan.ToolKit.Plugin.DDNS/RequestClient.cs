using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Text.Json;
using System.Threading.Tasks;
using Newtonsoft.Json;
using TomDan.ToolKit.Plugin.DDNS;
namespace TomDan.ToolKit.Plugin.DDNS
{

    public class RequestClient
    {
        string SecretId { get; set; }
        string SecretKey { get; set; }
        string Host { get; set; }
        string Service { get; set; }
        string Region { get; set; }
        string Version { get; set; }

        public RequestClient(string secretId, string secretKey, string host, string service)
        {
            SecretId = secretId;
            SecretKey = secretKey;
            Host = host;
            Service = service;
            Region = string.Empty;
            Version = "2021-03-23";
        }
        public async Task<(bool, TcResponse<R>)> Send<T, R>(string action, T payload)
        {

            try
            {
                // 获取当前时间
                var now = DateTime.UtcNow;

                // 将 payload 序列化为 JSON 字符串
                var payloadJson = JsonConvert.SerializeObject(payload);

                // 获取时间戳和日期
                var timestamp = new DateTimeOffset(now).ToUnixTimeSeconds();
                var date = now.ToString("yyyy-MM-dd", CultureInfo.InvariantCulture);

                // 计算认证信息
                var authorization = MakePostAuthorization(timestamp, date, payloadJson);
                // 设置请求头
                var request = new HttpRequestMessage(HttpMethod.Post, $"https://{Host}")
                {
                    Content = new StringContent(payloadJson, Encoding.UTF8, "application/json")
                };
                request.Headers.Add("Authorization", authorization);
                request.Headers.Add("X-TC-Action", action);
                request.Headers.Add("X-TC-Timestamp", timestamp.ToString());
                request.Headers.Add("X-TC-Version", Version);
                request.Headers.Add("X-TC-Region", Region);
                var client = new HttpClient();
                var response = await client.SendAsync(request);
                // 确保请求成功
                response.EnsureSuccessStatusCode();

                // 读取并反序列化响应
                var jsonResponse = await response.Content.ReadAsStringAsync();
                var result = System.Text.Json.JsonSerializer.Deserialize<TcResponse<R>>(jsonResponse);
                return (true, result);
            }
            catch (Exception ex)
            {
                return (false, null);
            }
        }
        public string MakePostAuthorization(long timestamp, string date, string payload)
        {
            /* first */
            string httpRequestMethod = "POST";
            string canonicalUri = "/";
            string canonicalQueryString = "";
            string canonicalHeaders = $"content-type:application/json; charset=utf-8\nhost:{Host}\n";
            string signedHeaders = "content-type;host";
            string hashedRequestPayload = ComputeSha256Hash(payload);

            string canonicalRequest = $"{httpRequestMethod}\n{canonicalUri}\n{canonicalQueryString}\n{canonicalHeaders}\n{signedHeaders}\n{hashedRequestPayload}";

            /* second */
            string algorithm = "TC3-HMAC-SHA256";
            string credentialScope = $"{date}/{Service}/tc3_request";
            string hashedCanonicalRequest = ComputeSha256Hash(canonicalRequest);

            string stringToSign = $"{algorithm}\n{timestamp}\n{credentialScope}\n{hashedCanonicalRequest}";

            /* third */
            byte[] secretDate = HmacSha256(Encoding.UTF8.GetBytes(date), Encoding.UTF8.GetBytes($"TC3{SecretKey}"));
            byte[] secretService = HmacSha256(Encoding.UTF8.GetBytes(Service), secretDate);
            byte[] secretSigning = HmacSha256(Encoding.UTF8.GetBytes("tc3_request"), secretService);

            string signature = ComputeHmacSha256Hex(Encoding.UTF8.GetBytes(stringToSign), secretSigning);

            /* forth */
            string authorization = $"TC3-HMAC-SHA256 Credential={SecretId}/{credentialScope},SignedHeaders={signedHeaders},Signature={signature}";
            return authorization;
        }

        private static string ComputeSha256Hash(string input)
        {
            using (var sha256 = SHA256.Create())
            {
                var bytes = Encoding.UTF8.GetBytes(input);
                var hash = sha256.ComputeHash(bytes);
                return BitConverter.ToString(hash).Replace("-", "").ToLower(); // Convert to hex string
            }
        }

        private static byte[] HmacSha256(byte[] message, byte[] key)
        {
            using (var hmac = new HMACSHA256(key))
            {
                return hmac.ComputeHash(message); // Return raw byte array
            }
        }

        private static string ComputeHmacSha256Hex(byte[] message, byte[] key)
        {
            using (var hmac = new HMACSHA256(key))
            {
                var hash = hmac.ComputeHash(message);
                return BitConverter.ToString(hash).Replace("-", "").ToLower(); // Convert to hex string
            }
        }
    }
}
