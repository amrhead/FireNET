/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2009.

-------------------------------------------------------------------------
History:
- 15:05:2009   Created by Federico Rebora

*************************************************************************/

#ifndef COLOR_GRADIENT_MANAGER_H_INCLUDED
#define COLOR_GRADIENT_MANAGER_H_INCLUDED

#include <IColorGradingController.h>



namespace Graphics
{
    class CColorGradientManager
    {
    public:
        CColorGradientManager();

		void TriggerFadingColorGradient(const string& filePath, const float fadeInTimeInSeconds);

		void UpdateForThisFrame(const float frameTimeInSeconds);
		void Reset();
		void Serialize(TSerialize serializer);

	private:
		void FadeInLastLayer(const float frameTimeInSeconds);
		void FadeOutCurrentLayers();
		void RemoveZeroWeightedLayers();
		void SetLayersForThisFrame();
		void LoadGradients();

		IColorGradingController& GetColorGradingController();

	private:

		class LoadedColorGradient
		{
		public:
			LoadedColorGradient(const string& filePath, const SColorChartLayer& layer, const float fadeInTimeInSeconds);

		public:
			void FadeIn(const float frameTimeInSeconds);
			void FadeOut(const float blendAmountOfFadingInGradient);
			
			void FreezeMaximumBlendAmount();

			SColorChartLayer m_layer;
			string m_filePath;
			float m_fadeInTimeInSeconds;
			float m_elapsedTime;
			float m_maximumBlendAmount;
		};

		class LoadingColorGradient
		{
		public:
			LoadingColorGradient(const string& filePath, const float fadeInTimeInSeconds);

			LoadedColorGradient Load(IColorGradingController& colorGradingController) const;

		public:
			string m_filePath;
			float m_fadeInTimeInSeconds;
		};

	private:

		std::vector<LoadingColorGradient> m_colorGradientsToLoad;
		std::vector<LoadedColorGradient> m_currentGradients;
    };
}

#endif //COLOR_GRADIENT_MANAGER_H_INCLUDED