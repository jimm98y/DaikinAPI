namespace DaikinAPI.Model
{
    /// <summary>
    /// State of the Daikin AC.
    /// </summary>
    public class DaikinState
    {
        /// <summary>
        /// Timestamp in seconds since the Arduino controller started.
        /// </summary>
        public int Timestamp { get; set; }

        /// <summary>
        /// Temperature in degrees Celsius.
        /// </summary>
        public int Temperature { get; set; }

        /// <summary>
        /// Fan mode.
        /// </summary>
        public int Fan { get; set; }

        /// <summary>
        /// Is the AC on?
        /// </summary>
        public bool IsOn { get; set; }
        
        /// <summary>
        /// Is the AC swinging?
        /// </summary>
        public bool IsSwinging { get; set; }

        /// <summary>
        /// Is the on timer active?
        /// </summary>
        public bool IsOnTimerActive { get; set; }

        /// <summary>
        /// Value of the on timer.
        /// </summary>
        public int OnTimerValue { get; set; }

        /// <summary>
        /// Is the off timer active?
        /// </summary>
        public bool IsOffTimerActive { get; set; }

        /// <summary>
        /// Value of the off timer.
        /// </summary>
        public int OffTimerValue { get; set; }

        /// <summary>
        /// AC mode.
        /// </summary>
        public int Mode { get; set; }
    }
}
