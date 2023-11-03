#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
    Super minimal progress bar to monitor long running processes.
    Features:
    - shows the spent time and the estimated time till completion
    - progress bar adapts to length of terminal window

    ## usage example
    from pbar import ProgressBar
    rangevalue = 40

    progressbar = ProgressBar(rangevalue, 'explicit update')
    for new_value in range(rangevalue):
        .. do things..
        pbar.update(new_value)

    progressbar = ProgressBar(rangevalue, 'implicit update by 1')
    for i in range(rangevalue):
        .. do things..
        pbar.update()

    code taken from here: http://stackoverflow.com/questions/3173320/text-progress-bar-in-the-console?lq=1
    and here: http://pypi.python.org/pypi/progressbar/2.2

    get the newest version here: https://github.com/shackenberg/pbar.py
    by Ludwig Schmidt-Hackenberg
"""
import sys
from time import time, sleep
from datetime import timedelta
import os
import unittest


class ProgressBar:
    def __init__(
        self, max_value, title=None, start_state=0, max_refreshrate=0.3, zero_index=True
    ):
        self.max_value = max_value
        self.start_time = time()
        self.state = start_state
        self.old_length = self.determine_length_pbar()
        self.title = self.prepare_title(title)
        self.time_last_update = 0
        self.max_refreshrate = max_refreshrate
        self.is_last_update = False
        self.zero_index = zero_index
        output_string = self.build_output_string(time())
        self.print_pbar(output_string)

    def determine_length_pbar(self):
        try:
            return self.get_width_of_terminal()
        except:
            rule_of_thumb_standard_value = 80
            return rule_of_thumb_standard_value

    def get_width_of_terminal(self):
        _, ncolumns = os.popen("stty size", "r").read().split()
        width = int(ncolumns)
        return width

    def prepare_title(self, title):
        if title is None:
            return ""
        else:
            return title + " "

    def update(self, current_value=None):
        if current_value is None:
            self.state += 1
        else:
            if self.zero_index:
                self.state = current_value + 1
            else:
                self.state = current_value
        if self.state > self.max_value:
            # raise ValueError("state >= max_value")
            return
        current_time = time()
        time_since_last_update = current_time - self.time_last_update
        if self.state == self.max_value:
            self.is_last_update = True
        if (time_since_last_update > self.max_refreshrate) | self.is_last_update:
            output_string = self.build_output_string(current_time)
            self.print_pbar(output_string)
            self.time_last_update = current_time
        if self.is_last_update:
            self.jump_to_newline()

    def time_div_to_short_str(self, time_div):
        return str(timedelta(seconds=round(time_div)))

    def compute_bar_length(self, overhead, progress):
        max_bar_length = self.length - overhead
        return progress * max_bar_length / 100

    def compute_filling_length(self, overhead, progress):
        max_bar_length = self.length - overhead
        bar_length = progress * max_bar_length / 100
        return max_bar_length - bar_length

    def computed_estimate_time_left(self, complete_elapsed_time):
        estimated_time_left = complete_elapsed_time * (
            self.max_value / float(self.state) - 1
        )
        return estimated_time_left

    def build_output_string(self, current_time):
        self.length = self.determine_length_pbar()
        progress = int(round(self.state * 100.0 / self.max_value))
        complete_elapsed_time = current_time - self.start_time
        complete_elapsed_time_pretty = self.time_div_to_short_str(complete_elapsed_time)

        if (complete_elapsed_time > 3) & (self.state > 0):
            estimated_time_left = self.computed_estimate_time_left(
                complete_elapsed_time
            )
            estimated_time_left_pretty = self.time_div_to_short_str(estimated_time_left)
            estimated_time_left_pretty_formatted = (
                " - " + estimated_time_left_pretty + " remaining"
            )
        else:
            estimated_time_left_pretty_formatted = ""
        progress_str = " " + str(progress) + "% in "
        len_of_brackets = 2
        overhead = (
            len_of_brackets
            + len(progress_str)
            + len(complete_elapsed_time_pretty)
            + len(self.title)
            + len(estimated_time_left_pretty_formatted)
        )
        bar_length = self.compute_bar_length(overhead, progress)
        filling_length = self.compute_filling_length(overhead, progress)
        progressbar_string = (
            "[" + "#" * int(bar_length) + " " * int(filling_length) + "]"
        )

        complete_elapsed_time_pretty = str(complete_elapsed_time_pretty)

        if self.length == self.length:
            carriage_return = "\r"
        else:
            carriage_return = "\n"

        ordered_output_string_fields = [
            carriage_return,
            self.title,
            progressbar_string,
            progress_str,
            complete_elapsed_time_pretty,
            estimated_time_left_pretty_formatted,
        ]

        output_string = "".join(ordered_output_string_fields)

        self.old_length = self.length

        return output_string

    def print_pbar(self, output_string):
        sys.stdout.write(output_string)
        sys.stdout.flush()

    def jump_to_newline(self):
        print()


class ProgressBarTests(unittest.TestCase):
    """run tests with
    python -m unittest pbar
    """

    def testExplicitStart(self):
        pbar = ProgressBar(10)
        pbar.update(0)
        pbar.length = 80
        output = pbar.build_output_string(time())
        should_value = "\r[######                                                       ] 10% in 0:00:00"
        self.assertEqual(output, should_value)

    def testExplicitEnd(self):
        pbar = ProgressBar(99)
        pbar.update(98)
        pbar.length = 80
        output = pbar.build_output_string(time())
        should_value = "\r[#############################################################] 100% in 0:00:00"
        self.assertEqual(output, should_value)

    def testImplicitStart(self):
        pbar = ProgressBar(10)
        pbar.update()
        pbar.length = 80
        output = pbar.build_output_string(time())
        should_value = "\r[######                                                       ] 10% in 0:00:00"
        self.assertEqual(output, should_value)

    def testImplicitEnd(self):
        max_value = 99
        pbar = ProgressBar(max_value)
        for i in range(max_value - 1):
            pbar.update()
        pbar.update()
        pbar.length = 80
        output = pbar.build_output_string(time())
        should_value = "\r[#############################################################] 100% in 0:00:00"
        self.assertEqual(output, should_value)

    def testTitle(self):
        example_title = "example title"
        pbar = ProgressBar(10, example_title)
        pbar.update(0)
        pbar.length = 80
        output = pbar.build_output_string(time())
        should_value = "\rexample title [####                                           ] 10% in 0:00:00"
        self.assertEqual(output, should_value)

    def testElapsedTime(self):
        pbar = ProgressBar(10)
        pbar.update(0)
        sleep(1)
        pbar.length = 80
        output = pbar.build_output_string(time())
        should_value = "\r[######                                                       ] 10% in 0:00:01"
        self.assertEqual(output, should_value)

    def testEstimatedTime(self):
        pbar = ProgressBar(10)
        pbar.update(0)
        sleep(3)
        pbar.length = 80
        output = pbar.build_output_string(time())
        should_value = "\r[####                                     ] 10% in 0:00:03 - 0:00:27 remaining"
        self.assertEqual(output, should_value)

    def testEstimatedTimeEnd(self):
        pbar = ProgressBar(10)
        pbar.update(0)
        sleep(3)
        pbar.update(9)
        pbar.length = 80
        output = pbar.build_output_string(time())
        should_value = "\r[#########################################] 100% in 0:00:03 - 0:00:00 remaining"
        self.assertEqual(output, should_value)

    def testException(self):
        max_value = 99
        pbar = ProgressBar(max_value)
        self.assertRaises(ValueError, pbar.update, max_value)


if __name__ == "__main__":
    ## usage examples
    example_rangevalue = 40
    example_title_example_one = "explicit update"
    progressbar = ProgressBar(example_rangevalue, example_title_example_one)
    for new_value in range(example_rangevalue):
        progressbar.update(new_value)
        sleep(0.1)

    example_title_example_two = "implicit update by 1"
    progressbar = ProgressBar(example_rangevalue, example_title_example_two)
    for i in range(example_rangevalue):
        sleep(0.1)
        progressbar.update()

    example_title_example_three = "slow updates"
    example_rangevalue_three = 4
    progressbar = ProgressBar(example_rangevalue_three, example_title_example_three)
    for i in range(example_rangevalue_three):
        sleep(3.8)
        progressbar.update()

    example_title_example_three = "using zero_index=False"
    example_rangevalue = 40
    progressbar = ProgressBar(
        example_rangevalue, example_title_example_three, zero_index=False
    )
    foo = list()
    for i in range(example_rangevalue):
        progressbar.update(len(foo))
        foo.append(i)
        progressbar.update(len(foo))
        sleep(0.6)
