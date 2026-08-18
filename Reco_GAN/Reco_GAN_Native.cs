/*
 * ReconSage_Cli - Advanced Network & Telemetry Reconnaissance Framework
 * Copyright (C) 2026 ReconSage_Cli Authors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
using System.Runtime.InteropServices;
using ReconSageLogger;

namespace Reco_GAN_Native
{
    public class Reco_GAN
    {
        [DllImport("reco_gan_ml_module_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern void RecoGAN_Predict(string domain, double k_factor);
        [DllImport("reco_gan_ml_module_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern void reco_gan_training_data_save(string domain, double k_factor);
        private string domain = string.Empty;
        private double k_factor;
        public Reco_GAN(string _domain, double _k_factor)
        {
            domain = _domain;
            k_factor = _k_factor;
        }

        public void Training()
        {
            Logger.Info("RecoGAN Training starts.....");
            reco_gan_training_data_save(domain, k_factor);
            Logger.Success("RecoGAN Training ends visit the Reco_GAN_Data/ Directory in the home directory");
        }
        public void Predict()
        {
            Logger.Info("RecoGAN prediction starts.....");
            RecoGAN_Predict(domain, k_factor);
            Logger.Success("RecoGAN prediction ends.....");
        }
    }
}