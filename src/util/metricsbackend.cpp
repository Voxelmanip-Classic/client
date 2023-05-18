/*
Minetest
Copyright (C) 2013-2020 Minetest core developers team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "metricsbackend.h"
#include "util/thread.h"

/* Plain implementation */

class SimpleMetricCounter : public MetricCounter
{
public:
	SimpleMetricCounter() : MetricCounter(), m_counter(0.0) {}

	virtual ~SimpleMetricCounter() {}

	void increment(double number) override
	{
		MutexAutoLock lock(m_mutex);
		m_counter += number;
	}
	double get() const override
	{
		MutexAutoLock lock(m_mutex);
		return m_counter;
	}

private:
	mutable std::mutex m_mutex;
	double m_counter;
};

class SimpleMetricGauge : public MetricGauge
{
public:
	SimpleMetricGauge() : MetricGauge(), m_gauge(0.0) {}

	virtual ~SimpleMetricGauge() {}

	void increment(double number) override
	{
		MutexAutoLock lock(m_mutex);
		m_gauge += number;
	}
	void decrement(double number) override
	{
		MutexAutoLock lock(m_mutex);
		m_gauge -= number;
	}
	void set(double number) override
	{
		MutexAutoLock lock(m_mutex);
		m_gauge = number;
	}
	double get() const override
	{
		MutexAutoLock lock(m_mutex);
		return m_gauge;
	}

private:
	mutable std::mutex m_mutex;
	double m_gauge;
};

MetricCounterPtr MetricsBackend::addCounter(
		const std::string &name, const std::string &help_str, Labels labels)
{
	return std::make_shared<SimpleMetricCounter>();
}

MetricGaugePtr MetricsBackend::addGauge(
		const std::string &name, const std::string &help_str, Labels labels)
{
	return std::make_shared<SimpleMetricGauge>();
}
