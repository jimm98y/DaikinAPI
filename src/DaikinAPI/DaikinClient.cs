using DaikinAPI.API;
using DaikinAPI.Model;
using System;
using System.Net.Http;
using System.Threading.Tasks;

namespace DaikinAPI
{
    /// <summary>
    /// Client for the Arduino Daikin AC controller.
    /// </summary>
    public class DaikinClient : IDisposable
    {
        private HttpClient _httpClient;
        private readonly string _arduinoIpAddress;
        private readonly string _arduinoServiceAddress;

        /// <summary>
        /// Ctor.
        /// </summary>
        /// <param name="arduinoIpAddress">IP address of the Arduino controller. Currently hard-coded in the Arduino program.</param>
        /// <exception cref="ArgumentNullException"></exception>
        public DaikinClient(string arduinoIpAddress)
        {
            _httpClient = new HttpClient();
            _arduinoIpAddress = arduinoIpAddress ?? throw new ArgumentNullException(nameof(arduinoIpAddress));
            _arduinoServiceAddress = $"http://{_arduinoIpAddress}";
        }

        /// <summary>
        /// Get the current state of the Daikin AC from Arduino controller.
        /// </summary>
        /// <returns><see cref="DaikinState"/>.</returns>
        public async Task<DaikinState> GetStateAsync()
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
                IsOnTimerActive = deserialized.TimerOn != 0,
                OnTimerValue = deserialized.TimerOnValue,
                IsOffTimerActive = deserialized.TimerOff != 0,
                OffTimerValue = deserialized.TimerOffValue,
            };
        }

        /// <summary>
        /// Set the desired state of the Daikin AC.
        /// </summary>
        /// <param name="state"><see cref="DaikinState"/>.</param>
        /// <returns><see cref="Task"/>.</returns>
        public async Task SetStateAsync(DaikinState state)
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

        #region IDisposable

        private bool disposedValue;

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
