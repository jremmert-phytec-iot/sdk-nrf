/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "sensor.h"

#include <stdint.h>

#include <temp_sensor_manager.h>

#include <DeviceInfoProviderImpl.h>

#include <app-common/zap-generated/ids/Attributes.h>
#include <controller/ReadInteraction.h>
#include <zephyr/logging/log.h>

using namespace ::chip;
using namespace ::chip::DeviceLayer;
using namespace ::chip::app;

LOG_MODULE_DECLARE(app);

namespace
{

constexpr chip::EndpointId mTemperatureMeasurementEndpointId = 1;

} // namespace

RealSensor::RealSensor()
{
	BindingHandler::Init();
}

void RealSensor::TemperatureMeasurement()
{
	BindingHandler::BindingData *data = Platform::New<BindingHandler::BindingData>();
	data->ClusterId = Clusters::TemperatureMeasurement::Id;
	data->EndpointId = mTemperatureMeasurementEndpointId;
	data->IsGroup = BindingHandler::IsGroupBound();
	data->InvokeCommandFunc = TemperatureMeasurementReadHandler;
	BindingHandler::RunBoundClusterAction(data);
}

void RealSensor::TemperatureMeasurementReadHandler(const EmberBindingTableEntry &binding,
						   OperationalDeviceProxy *deviceProxy,
						   BindingHandler::BindingData &bindingData)
{
	auto onSuccess = [dataPointer = Platform::New<BindingHandler::BindingData>(bindingData)](
				 const ConcreteDataAttributePath &attributePath, const auto &dataResponse) {
		ChipLogProgress(NotSpecified, "Read Temperature Sensor attribute succeeded");

		VerifyOrReturn(!(dataResponse.IsNull()), LOG_ERR("Device invalid");
			       Platform::Delete<BindingHandler::BindingData>(dataPointer););

		int16_t responseValue = dataResponse.Value();

		TempSensorManager::Instance().SetLocalTemperature(responseValue);
		BindingHandler::OnInvokeCommandSucces(dataPointer);
	};

	auto onFailure = [](const ConcreteDataAttributePath *attributePath, CHIP_ERROR error) {
		ChipLogError(NotSpecified, "Read Temperature Sensor attribute failed: %" CHIP_ERROR_FORMAT,
			     error.Format());
	};

	VerifyOrReturn(deviceProxy != nullptr && deviceProxy->ConnectionReady(), LOG_ERR("Device invalid"));

	Controller::ReadAttribute<Clusters::TemperatureMeasurement::Attributes::MeasuredValue::TypeInfo>(
		deviceProxy->GetExchangeManager(), deviceProxy->GetSecureSession().Value(), binding.remote, onSuccess,
		onFailure);
}
