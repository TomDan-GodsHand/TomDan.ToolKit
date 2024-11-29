using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Net.Http.Headers;
using System.Security.Cryptography;
using System.Text;
using System.Text.Json;
using System.Threading.Tasks;
using Microsoft.Extensions.Logging;
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
                var date = now.ToString("yyyy-MM-dd", CultureInfo.InvariantCulture);
                var url = "https://" + Host;
                var contentType = "application/json; charset=utf-8";
                var timestamp = ((int)DateTime.UtcNow.Subtract(new DateTime(1970, 1, 1)).TotalSeconds).ToString();
                // 计算认证信息
                var auth = GetAuth(SecretId, SecretKey, Host, contentType, timestamp, payloadJson);
                // 设置请求头
                var request = new HttpRequestMessage();
                request.Method = HttpMethod.Post;
                request.Headers.Add("Host", Host);
                request.Headers.Add("X-TC-Timestamp", timestamp);
                request.Headers.Add("X-TC-Version", Version);
                request.Headers.Add("X-TC-Action", action);
                request.Headers.Add("X-TC-Region", Region);
                request.Headers.Add("X-TC-Token", "");
                request.Headers.Add("X-TC-RequestClient", "SDK_NET_BAREBONE");
                request.Headers.TryAddWithoutValidation("Authorization", auth);

                request.RequestUri = new Uri(url);
                request.Content = new StringContent(payloadJson, MediaTypeWithQualityHeaderValue.Parse(contentType));
                var client = new HttpClient();
                var response = await client.SendAsync(request);
                // 确保请求成功
                response.EnsureSuccessStatusCode();

                // 读取并反序列化响应
                var jsonResponse = await response.Content.ReadAsStringAsync();
                var result = JsonConvert.DeserializeObject<TcResponse<R>>(jsonResponse);
                return (true, result);
            }
            catch (Exception ex)
            {
                throw ex;
            }
        }
        string GetAuth(string secretId, string secretKey, string host, string contentType, string timestamp, string body)
        {
            var canonicalURI = "/";
            var canonicalHeaders = "content-type:" + contentType + "\nhost:" + host + "\n";
            var signedHeaders = "content-type;host";
            var hashedRequestPayload = Sha256Hex(body);
            var canonicalRequest = "POST" + "\n"
                                          + canonicalURI + "\n"
                                          + "\n"
                                          + canonicalHeaders + "\n"
                                          + signedHeaders + "\n"
                                          + hashedRequestPayload;

            var algorithm = "TC3-HMAC-SHA256";
            var date = new DateTime(1970, 1, 1, 0, 0, 0, 0, DateTimeKind.Utc).AddSeconds(int.Parse(timestamp))
                .ToString("yyyy-MM-dd");
            var service = host.Split(".")[0];
            var credentialScope = date + "/" + service + "/" + "tc3_request";
            var hashedCanonicalRequest = Sha256Hex(canonicalRequest);
            var stringToSign = algorithm + "\n"
                                         + timestamp + "\n"
                                         + credentialScope + "\n"
                                         + hashedCanonicalRequest;

            var tc3SecretKey = Encoding.UTF8.GetBytes("TC3" + secretKey);
            var secretDate = HmacSha256(tc3SecretKey, Encoding.UTF8.GetBytes(date));
            var secretService = HmacSha256(secretDate, Encoding.UTF8.GetBytes(service));
            var secretSigning = HmacSha256(secretService, Encoding.UTF8.GetBytes("tc3_request"));
            var signatureBytes = HmacSha256(secretSigning, Encoding.UTF8.GetBytes(stringToSign));
            var signature = BitConverter.ToString(signatureBytes).Replace("-", "").ToLower();

            return algorithm + " "
                             + "Credential=" + secretId + "/" + credentialScope + ", "
                             + "SignedHeaders=" + signedHeaders + ", "
                             + "Signature=" + signature;
        }

        private byte[] HmacSha256(byte[] key, byte[] msg)
        {
            using (HMACSHA256 mac = new HMACSHA256(key))
            {
                return mac.ComputeHash(msg);
            }
        }

        private string Sha256Hex(string s)
        {
            using (SHA256 algo = SHA256.Create())
            {
                byte[] hashbytes = algo.ComputeHash(Encoding.UTF8.GetBytes(s));
                StringBuilder builder = new StringBuilder();
                for (int i = 0; i < hashbytes.Length; ++i)
                {
                    builder.Append(hashbytes[i].ToString("x2"));
                }

                return builder.ToString();
            }
        }
    }
}
