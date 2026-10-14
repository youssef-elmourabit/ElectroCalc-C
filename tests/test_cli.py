"""Teste le véritable exécutable ; uniquement la bibliothèque standard Python."""

from pathlib import Path
import subprocess
import sys
import unittest

BINARY = Path(sys.argv.pop(1) if len(sys.argv) > 1 else "build/electrocalc").resolve()


class ConsoleTests(unittest.TestCase):
    def run_app(self, entries):
        result = subprocess.run(
            [str(BINARY)], input=entries, capture_output=True,
            text=True, encoding="utf-8", timeout=5, check=False,
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stderr, "")
        return result.stdout

    def test_100_ohms_20_milliamps(self):
        self.assertIn("Résultat : U = 2 V", self.run_app("1\n100\n0.02\n0\n"))

    def test_220_ohms_50_milliamps(self):
        self.assertIn("Résultat : U = 11 V", self.run_app("1\n220\n0.05\n0\n"))

    def test_zero_current(self):
        self.assertIn("Résultat : U = 0 V", self.run_app("1\n100\n0\n0\n"))

    def test_negative_resistance_then_recovery(self):
        output = self.run_app("1\n-100\n100\n0.02\n0\n")
        self.assertIn("résistance doit être strictement positive", output)
        self.assertIn("Résultat : U = 2 V", output)

    def test_zero_resistance_then_recovery(self):
        output = self.run_app("1\n0\n100\n0.02\n0\n")
        self.assertIn("résistance doit être strictement positive", output)
        self.assertIn("Résultat : U = 2 V", output)

    def test_nonnumeric_resistance(self):
        output = self.run_app("1\nabc\n100\n0.02\n0\n")
        self.assertIn("nombre fini valide", output)
        self.assertIn("Résultat : U = 2 V", output)

    def test_nonnumeric_current(self):
        output = self.run_app("1\n100\nabc\n0.02\n0\n")
        self.assertIn("nombre fini valide", output)
        self.assertIn("Résultat : U = 2 V", output)

    def test_invalid_menu_choices(self):
        for choice in ("9", "-1", "abc", "1.0", "1x", "", "9" * 100):
            with self.subTest(choice=choice):
                self.assertIn("choix de menu invalide", self.run_app(choice + "\n0\n"))

    def test_new_features_are_active(self):
        for entries, expected in (("4\n12\n1000\n2000\n0\n", "U_sortie = 8 V"),
                                  ("5\n12\n2\n0\n", "P = 24 W")):
            with self.subTest(entries=entries):
                output = self.run_app(entries)
                self.assertNotIn("À développer", output)
                self.assertIn("Résultat : " + expected, output)

    def test_return_to_menu_and_second_calculation(self):
        output = self.run_app("1\n100\n0.02\n1\n220\n0.05\n0\n")
        self.assertIn("Résultat : U = 2 V", output)
        self.assertIn("Résultat : U = 11 V", output)
        self.assertEqual(output.count("Retour au menu."), 2)

    def test_quit(self):
        self.assertIn("Au revoir !", self.run_app("0\n"))

    def test_eof_at_menu(self):
        self.assertIn("Fin de saisie", self.run_app(""))

    def test_eof_during_resistance(self):
        self.assertIn("Fin de saisie", self.run_app("1\n"))

    def test_eof_during_current(self):
        self.assertIn("Fin de saisie", self.run_app("1\n100\n"))

    def test_last_line_without_newline(self):
        self.assertIn("Résultat : U = 2 V", self.run_app("1\n100\n0.02"))

    def test_signed_current(self):
        self.assertIn("Résultat : U = -2 V", self.run_app("1\n100\n-0.02\n0\n"))

    def test_scientific_notation_and_spaces(self):
        self.assertIn("Résultat : U = 2 V", self.run_app(" 1 \n 1e2 \n 2e-2 \n0\n"))

    def test_invalid_numeric_values(self):
        for invalid in ("nan", "inf", "-inf", "1e9999", "1e-9999", "0,02", "10 Ω", ""):
            for field in ("resistance", "current"):
                with self.subTest(value=invalid, field=field):
                    entries = (f"1\n{invalid}\n100\n0.02\n0\n" if field == "resistance"
                               else f"1\n100\n{invalid}\n0.02\n0\n")
                    output = self.run_app(entries)
                    self.assertIn("nombre fini valide", output)
                    self.assertIn("Résultat : U = 2 V", output)

    def test_multiplication_overflow(self):
        output = self.run_app("1\n1e308\n10\n0\n")
        self.assertIn("résultat hors de la plage", output)
        self.assertNotIn("Résultat :", output)

    def test_long_menu_line_is_consumed(self):
        output = self.run_app("1" * 300 + "\n0\n")
        self.assertEqual(output.count("choix de menu invalide"), 1)

    def test_long_numeric_line_is_consumed(self):
        output = self.run_app("1\n" + "1" * 300 + "\n100\n0.02\n0\n")
        self.assertEqual(output.count("nombre fini valide"), 1)
        self.assertIn("Résultat : U = 2 V", output)

    def test_embedded_null_is_rejected(self):
        output = self.run_app("1\n100\x00abc\n100\n0.02\n0\n")
        self.assertIn("nombre fini valide", output)
        self.assertIn("Résultat : U = 2 V", output)

    def test_series_resistance(self):
        output = self.run_app("2\n100\n220\n0\n")
        self.assertIn("Résultat : R_eq = 320 Ω (R1 = 100 Ω, R2 = 220 Ω)", output)

    def test_parallel_resistance(self):
        output = self.run_app("3\n100\n220\n0\n")
        self.assertIn("Résultat : R_eq = 68.75 Ω (R1 = 100 Ω, R2 = 220 Ω)", output)

    def test_equal_and_decimal_resistances(self):
        for choice, r1, r2, expected in ((2, "100", "100", "200"),
                                         (3, "100", "100", "50"),
                                         (2, "0.1", "0.2", "0.3"),
                                         (3, "0.1", "0.2", "0.06666666667")):
            with self.subTest(choice=choice, r1=r1, r2=r2):
                self.assertIn(f"Résultat : R_eq = {expected} Ω",
                              self.run_app(f"{choice}\n{r1}\n{r2}\n0\n"))

    def test_resistances_reject_zero_and_negative_values(self):
        for choice, expected in ((2, "320"), (3, "68.75")):
            for invalid in ("0", "-0", "-100"):
                for field in (1, 2):
                    with self.subTest(choice=choice, invalid=invalid, field=field):
                        entries = (f"{choice}\n{invalid}\n100\n220\n0\n" if field == 1
                                   else f"{choice}\n100\n{invalid}\n220\n0\n")
                        output = self.run_app(entries)
                        self.assertEqual(output.count("résistance doit être strictement positive"), 1)
                        self.assertIn(f"Résultat : R_eq = {expected} Ω", output)

    def test_resistances_reject_invalid_numbers(self):
        invalid_values = ("abc", "100abc", "0,02", "", "nan", "inf", "-inf",
                          "1e9999", "1e-9999", "100 Ω", "100\x00abc", "1" * 300)
        for choice, expected in ((2, "320"), (3, "68.75")):
            for invalid in invalid_values:
                for field in (1, 2):
                    with self.subTest(choice=choice, invalid=invalid, field=field):
                        entries = (f"{choice}\n{invalid}\n100\n220\n0\n" if field == 1
                                   else f"{choice}\n100\n{invalid}\n220\n0\n")
                        output = self.run_app(entries)
                        self.assertEqual(output.count("nombre fini valide"), 1)
                        self.assertIn(f"Résultat : R_eq = {expected} Ω", output)

    def test_series_overflow_and_recovery(self):
        output = self.run_app("2\n1e308\n1e308\n2\n100\n220\n0\n")
        self.assertEqual(output.count("résistance équivalente hors de la plage"), 1)
        self.assertEqual(output.count("Résultat : R_eq"), 1)
        self.assertIn("Résultat : R_eq = 320 Ω", output)

    def test_series_overflow_is_symmetric(self):
        largest = "1.7976931348623157e308"
        for r1, r2 in ((largest, "1"), ("1", largest)):
            with self.subTest(r1=r1, r2=r2):
                output = self.run_app(f"2\n{r1}\n{r2}\n0\n")
                self.assertIn("résistance équivalente hors de la plage", output)
                self.assertNotIn("Résultat :", output)

    def test_parallel_extreme_values(self):
        for r1, r2, expected in (("1e308", "1e308", "5e+307"),
                                 ("1e-200", "1e-200", "5e-201"),
                                 ("1e-200", "1e200", "1e-200"),
                                 ("1e200", "1e-200", "1e-200"),
                                 ("4e-308", "4e-308", "2e-308")):
            with self.subTest(r1=r1, r2=r2):
                output = self.run_app(f"3\n{r1}\n{r2}\n0\n")
                self.assertIn(f"Résultat : R_eq = {expected} Ω", output)
                self.assertNotIn("Erreur :", output)

    def test_resistances_scientific_notation_and_spaces(self):
        for choice, expected in ((2, "320"), (3, "68.75")):
            with self.subTest(choice=choice):
                output = self.run_app(f" {choice} \n 1e2 \n 2.2e2 \n0\n")
                self.assertIn(f"Résultat : R_eq = {expected} Ω", output)

    def test_resistances_eof_during_either_question(self):
        for choice in (2, 3):
            for entries in (f"{choice}\n", f"{choice}\n100\n"):
                with self.subTest(choice=choice, entries=entries):
                    output = self.run_app(entries)
                    self.assertIn("Fin de saisie", output)
                    self.assertNotIn("Résultat :", output)

    def test_resistances_last_line_without_newline(self):
        for choice, expected in ((2, "320"), (3, "68.75")):
            with self.subTest(choice=choice):
                output = self.run_app(f"{choice}\n100\n220")
                self.assertIn(f"Résultat : R_eq = {expected} Ω", output)
                self.assertIn("Fin de saisie", output)

    def test_all_available_calculations_in_one_session(self):
        output = self.run_app("2\n100\n220\n3\n100\n220\n1\n100\n0.02\n"
                              "4\n12\n1000\n2000\n5\n12\n2\n0\n")
        self.assertIn("Résultat : R_eq = 320 Ω", output)
        self.assertIn("Résultat : R_eq = 68.75 Ω", output)
        self.assertIn("Résultat : U = 2 V", output)
        self.assertIn("Résultat : U_sortie = 8 V", output)
        self.assertIn("Résultat : P = 24 W", output)
        self.assertEqual(output.count("Retour au menu."), 5)

    def test_menu_lists_available_features(self):
        output = self.run_app("0\n")
        self.assertIn("2. Deux résistances en série", output)
        self.assertIn("3. Deux résistances en parallèle", output)
        self.assertIn("4. Diviseur de tension", output)
        self.assertIn("5. Puissance électrique", output)
        self.assertNotIn("À développer", output)

    def test_divider_reference_values(self):
        for voltage, r1, r2, expected in (("12", "1000", "2000", "8"),
                                          ("10", "1000", "1000", "5"),
                                          ("0", "1000", "2000", "0"),
                                          ("-12", "1000", "2000", "-8"),
                                          ("12", "2000", "1000", "4")):
            with self.subTest(voltage=voltage, r1=r1, r2=r2):
                output = self.run_app(f"4\n{voltage}\n{r1}\n{r2}\n0\n")
                self.assertIn(f"Résultat : U_sortie = {expected} V", output)
                self.assertIn("idéal non chargé", output)
                self.assertNotIn("Erreur :", output)

    def test_power_reference_values_and_signs(self):
        for voltage, current, expected in (("12", "2", "24"), ("5", "0.5", "2.5"),
                                           ("0", "2", "0"), ("12", "0", "0"),
                                           ("-12", "2", "-24"), ("12", "-2", "-24"),
                                           ("-12", "-2", "24")):
            with self.subTest(voltage=voltage, current=current):
                output = self.run_app(f"5\n{voltage}\n{current}\n0\n")
                self.assertIn(f"Résultat : P = {expected} W", output)
                self.assertIn("Convention récepteur", output)
                self.assertNotIn("Erreur :", output)

    def test_new_calculations_reject_invalid_numbers(self):
        invalid_values = ("abc", "12abc", "0,5", "", "nan", "inf", "-inf",
                          "1e9999", "1e-9999", "12 V", "12\x00abc", "1" * 300)
        for choice, values, expected in ((4, ["12", "1000", "2000"], "U_sortie = 8 V"),
                                          (5, ["12", "2"], "P = 24 W")):
            for field in range(len(values)):
                for invalid in invalid_values:
                    with self.subTest(choice=choice, field=field, invalid=invalid):
                        entries = values[:field] + [invalid] + values[field:]
                        output = self.run_app(f"{choice}\n" + "\n".join(entries) + "\n0\n")
                        self.assertEqual(output.count("nombre fini valide"), 1)
                        self.assertIn("Résultat : " + expected, output)

    def test_divider_rejects_nonpositive_resistances(self):
        for field in (1, 2):
            for invalid in ("0", "-0", "-1000"):
                with self.subTest(field=field, invalid=invalid):
                    values = ["12", "1000", "2000"]
                    entries = values[:field] + [invalid] + values[field:]
                    output = self.run_app("4\n" + "\n".join(entries) + "\n0\n")
                    self.assertEqual(output.count("résistance doit être strictement positive"), 1)
                    self.assertIn("Résultat : U_sortie = 8 V", output)

    def test_divider_extreme_values(self):
        largest = "1.7976931348623157e308"
        cases = (("12", "1e308", "1e308", "6"),
                 (largest, largest, largest, "8.988465674e+307"),
                 (largest, "4e-308", largest, "1.797693135e+308"),
                 ("1e308", "1e308", "1e-200", "1e-200"),
                 ("1e308", "1e308", "4e-308", "4e-308"),
                 ("4e-308", "1", "1", "2e-308"),
                 ("-1e308", "1e308", "4e-308", "-4e-308"))
        for voltage, r1, r2, expected in cases:
            with self.subTest(voltage=voltage, r1=r1, r2=r2):
                output = self.run_app(f"4\n{voltage}\n{r1}\n{r2}\n0\n")
                self.assertIn(f"Résultat : U_sortie = {expected} V", output)
                self.assertNotIn("Erreur :", output)

    def test_divider_underflow_and_recovery(self):
        for voltage in ("1e-200", "-1e-200"):
            with self.subTest(voltage=voltage):
                output = self.run_app(f"4\n{voltage}\n1e200\n1e-200\n"
                                      "4\n12\n1000\n2000\n0\n")
                self.assertEqual(output.count("tension de sortie hors de la plage"), 1)
                self.assertEqual(output.count("Résultat : U_sortie"), 1)
                self.assertIn("Résultat : U_sortie = 8 V", output)

    def test_power_overflow_and_recovery(self):
        for voltage, current in (("1e308", "10"), ("10", "1e308"),
                                  ("-1e308", "10"), ("1e308", "-10"),
                                  ("-1e308", "-10")):
            with self.subTest(voltage=voltage, current=current):
                output = self.run_app(f"5\n{voltage}\n{current}\n5\n12\n2\n0\n")
                self.assertEqual(output.count("puissance hors de la plage"), 1)
                self.assertEqual(output.count("Résultat : P"), 1)
                self.assertIn("Résultat : P = 24 W", output)

    def test_power_underflow(self):
        for voltage in ("1e-200", "-1e-200"):
            with self.subTest(voltage=voltage):
                output = self.run_app(f"5\n{voltage}\n1e-200\n0\n")
                self.assertIn("puissance hors de la plage", output)
                self.assertNotIn("Résultat :", output)

    def test_power_representable_extremes(self):
        largest = "1.7976931348623157e308"
        for voltage, current, expected in ((largest, "1", "1.797693135e+308"),
                                           ("4e-308", "0.5", "2e-308"),
                                           ("1e-200", "1e200", "1"),
                                           (largest, "0", "0")):
            with self.subTest(voltage=voltage, current=current):
                output = self.run_app(f"5\n{voltage}\n{current}\n0\n")
                self.assertIn(f"Résultat : P = {expected} W", output)
                self.assertNotIn("Erreur :", output)

    def test_new_calculations_scientific_notation_and_spaces(self):
        for entries, expected in ((" 4 \n 1.2e1 \n 1e3 \n 2e3 \n0\n", "U_sortie = 8 V"),
                                  (" 5 \n 5e0 \n 5e-1 \n0\n", "P = 2.5 W")):
            with self.subTest(entries=entries):
                self.assertIn("Résultat : " + expected, self.run_app(entries))

    def test_new_calculations_eof_at_each_question(self):
        for choice, values in ((4, ["12", "1000", "2000"]), (5, ["12", "2"])):
            for count in range(len(values)):
                with self.subTest(choice=choice, count=count):
                    entries = "\n".join([str(choice)] + values[:count]) + "\n"
                    output = self.run_app(entries)
                    self.assertIn("Fin de saisie", output)
                    self.assertNotIn("Résultat :", output)

    def test_new_calculations_last_line_without_newline(self):
        for entries, expected in (("4\n12\n1000\n2000", "U_sortie = 8 V"),
                                  ("5\n12\n2", "P = 24 W")):
            with self.subTest(entries=entries):
                output = self.run_app(entries)
                self.assertIn("Résultat : " + expected, output)
                self.assertIn("Fin de saisie", output)


if __name__ == "__main__":
    unittest.main(verbosity=2)
