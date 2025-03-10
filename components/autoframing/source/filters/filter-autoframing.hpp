// AUTOGENERATED COPYRIGHT HEADER START
// Copyright (C) 2021-2023 Michael Fabian 'Xaymar' Dirks <info@xaymar.com>
// AUTOGENERATED COPYRIGHT HEADER END

#pragma once
#include "gfx/gfx-util.hpp"
#include "obs/gs/gs-rendertarget.hpp"
#include "obs/gs/gs-texture.hpp"
#include "obs/gs/gs-vertexbuffer.hpp"
#include "obs/obs-source-factory.hpp"
#include "plugin.hpp"
#include "util/util-threadpool.hpp"
#include "util/utility.hpp"

#include "warning-disable.hpp"
#include <atomic>
#include <list>
#include <memory>
#include <mutex>
#include "warning-enable.hpp"

#ifdef ENABLE_NVIDIA
#include "nvidia/ar/nvidia-ar-facedetection.hpp"
#endif

namespace streamfx::filter::autoframing {

	enum class tracking_mode : int64_t {
		SOLO  = 0,
		GROUP = 1,
	};

	enum class tracking_provider : int64_t {
		INVALID              = -1,
		AUTOMATIC            = 0,
		NVIDIA_FACEDETECTION = 1,
	};

	const char* cstring(tracking_provider provider);

	std::string string(tracking_provider provider);

	class autoframing_instance : public obs::source_instance {
		struct track_el {
			float age;
			vec2  pos;
			vec2  size;
			vec2  vel;
		};

		struct pred_el {
			// Motion-Predicted Position
			vec2 mp_pos;

			// Filtered Position
			streamfx::util::math::kalman1D<float> filter_pos_x;
			streamfx::util::math::kalman1D<float> filter_pos_y;

			// Offset Filtered Position
			vec2 offset_pos;

			// Padded Area
			vec2 pad_size;

			// Aspect-Ratio-Corrected Padded Area
			vec2 aspected_size;
		};

		bool                          _dirty;
		std::pair<uint32_t, uint32_t> _size;
		std::pair<uint32_t, uint32_t> _out_size;

		std::shared_ptr<::streamfx::gfx::util>              _gfx_debug;
		std::shared_ptr<::streamfx::obs::gs::effect>        _standard_effect;
		std::shared_ptr<::streamfx::obs::gs::rendertarget>  _input;
		std::shared_ptr<::streamfx::obs::gs::vertex_buffer> _vb;

		tracking_provider                       _provider;
		tracking_provider                       _provider_ui;
		std::atomic<bool>                       _provider_ready;
		std::mutex                              _provider_lock;
		std::shared_ptr<util::threadpool::task> _provider_task;

#ifdef ENABLE_NVIDIA
		std::shared_ptr<::streamfx::nvidia::ar::facedetection> _nvidia_fx;
#endif

		tracking_mode _track_mode;
		float         _track_frequency;

		float _motion_smoothing;
		float _motion_smoothing_kalman_pnc;
		float _motion_smoothing_kalman_mnc;
		float _motion_prediction;

		float _frame_stability;
		float _frame_stability_kalman;
		bool  _frame_padding_prc[2];
		vec2  _frame_padding;
		bool  _frame_offset_prc[2];
		vec2  _frame_offset;
		float _frame_aspect_ratio;

		float                                                         _track_frequency_counter;
		std::list<std::shared_ptr<track_el>>                          _tracked_elements;
		std::map<std::shared_ptr<track_el>, std::shared_ptr<pred_el>> _predicted_elements;

		streamfx::util::math::kalman1D<float> _frame_pos_x;
		streamfx::util::math::kalman1D<float> _frame_pos_y;
		vec2                                  _frame_pos;
		streamfx::util::math::kalman1D<float> _frame_size_x;
		streamfx::util::math::kalman1D<float> _frame_size_y;
		vec2                                  _frame_size;

		bool _debug;

		public:
		~autoframing_instance();
		autoframing_instance(obs_data_t* settings, obs_source_t* self);

		void load(obs_data_t* data) override;
		void migrate(obs_data_t* data, uint64_t version) override;
		void update(obs_data_t* data) override;
		void properties(obs_properties_t* properties);

		uint32_t get_width() override;
		uint32_t get_height() override;

		virtual void video_tick(float seconds) override;
		virtual void video_render(gs_effect_t* effect) override;

		private:
		void tracking_tick(float seconds);

		void switch_provider(tracking_provider provider);
		void task_switch_provider(util::threadpool::task_data_t data);

#ifdef ENABLE_NVIDIA
		void nvar_facedetection_load();
		void nvar_facedetection_unload();
		void nvar_facedetection_process();
		void nvar_facedetection_properties(obs_properties_t* props);
		void nvar_facedetection_update(obs_data_t* data);
#endif
	};

	class autoframing_factory : public obs::source_factory<streamfx::filter::autoframing::autoframing_factory, streamfx::filter::autoframing::autoframing_instance> {
#ifdef ENABLE_NVIDIA
		bool                                           _nvidia_available;
		std::shared_ptr<::streamfx::nvidia::cuda::obs> _nvcuda;
		std::shared_ptr<::streamfx::nvidia::cv::cv>    _nvcvi;
		std::shared_ptr<::streamfx::nvidia::ar::ar>    _nvar;
#endif

		public:
		autoframing_factory();
		~autoframing_factory() override;

		const char* get_name() override;

		void              get_defaults2(obs_data_t* data) override;
		obs_properties_t* get_properties2(autoframing_instance* data) override;

		static bool on_manual_open(obs_properties_t* props, obs_property_t* property, void* data);

		bool              is_provider_available(tracking_provider);
		tracking_provider find_ideal_provider();

		public: // Singleton
		static void                                 initialize();
		static void                                 finalize();
		static std::shared_ptr<autoframing_factory> instance();
	};
} // namespace streamfx::filter::autoframing
