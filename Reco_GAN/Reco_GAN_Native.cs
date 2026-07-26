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