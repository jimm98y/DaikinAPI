using DaikinAPI.API;
using DaikinAPI.Model;
using System;
using System.Net.Http;
using System.Threading.Tasks;

namespace DaikinAPI
{
    public class DaikinClient : IDisposable
    {
        private HttpClient _httpClient;
        private readonly string _arduinoIpAddress;
        private readonly string _arduinoServiceAddress;

        #region IDisposable

        private bool disposedValue;

        public DaikinClient(string arduinoIpAddress)
        {
            _httpClient = new HttpClient();
            _arduinoIpAddress = arduinoIpAddress ?? throw new ArgumentNullException(nameof(arduinoIpAddress));
            _arduinoServiceAddress = $"http://{_arduinoIpAddress}";
        }

        public async Task<DaikinState> GetState()
        {
            var response = await _httpClient.GetAsync(_arduinoServiceAddress);
            response.EnsureSuccessStatusCode();

            string json = await response.Content.ReadAsStringAsync();
            var deserialized = System.Text.Json.JsonSerializer.Deserialize<ReceivedDaikinState>(json);

            return new DaikinState()
            {
                Timestamp = deserialized.Timestamp,
                IsOn = deserialized.PowerState == 1,
                IsSwinging = deserialized.Swing == 1,
                Fan = deserialized.Fan,
                Mode = deserialized.Mode,
                Temperature = deserialized.Temperature,
                TimerOn = deserialized.TimerOn,
                TimerOnValue = deserialized.TimerOnValue,
                TimerOff = deserialized.TimerOff,
                TimerOffValue = deserialized.TimerOffValue,
            };
        }

        public async Task SetState(DaikinState state)
        {
            var response = await _httpClient.GetAsync(
                $"{_arduinoServiceAddress}" +
                $"?power={(state.IsOn ? "on" : "off")}"+
                $"&swing={(state.IsSwinging ? "on" : "off")}" +
                $"&temp={state.Temperature}" +
                $"&fan={state.Fan}" +
                $"&mode={state.Mode}"
                );
            response.EnsureSuccessStatusCode();
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!disposedValue)
            {
                if (disposing)
                {
                    if(_httpClient != null)
                    {
                        _httpClient.Dispose();
                        _httpClient = null;
                    }
                }

                disposedValue = true;
            }
        }

        public void Dispose()
        {
            Dispose(disposing: true);
            GC.SuppressFinalize(this);
        }

        #endregion // IDisposable
    }
}
