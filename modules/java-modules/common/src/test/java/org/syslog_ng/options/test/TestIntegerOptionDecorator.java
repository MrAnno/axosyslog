/*
 * Copyright (c) 2015 Balabit
 * Copyright (c) 2015 Viktor Juhasz <viktor.juhasz@balabit.com>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 *
 */

package org.syslog_ng.options.test;

import org.junit.Before;
import org.junit.Test;
import org.syslog_ng.options.IntegerOptionDecorator;
import org.syslog_ng.options.Option;
import org.syslog_ng.options.StringOption;

public class TestIntegerOptionDecorator extends TestOption {
	Option stringOption;
	Option intdecorator;
	@Before
	public void setUp() throws Exception {
		super.setUp();
		stringOption = new StringOption(owner, "port");
		intdecorator = new IntegerOptionDecorator(stringOption);

		options.put("name", "value");
	}

	@Test
	public void testNormal() {
		options.put("port", "9300");
		assertInitOptionSuccess(intdecorator);
	}

	@Test
	public void testInvalid() {
		options.put("port", "value");
		assertInitOptionFailed(intdecorator, "option port must be numerical");

		options.remove("port");
		assertInitOptionFailed(intdecorator, "option port must be numerical");
	}

}
