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

    def test_future_features(self):
        for choice in range(2, 6):
            with self.subTest(choice=choice):
                output = self.run_app(f"{choice}\n0\n")
                self.assertIn("À développer : fonctionnalité prévue", output)
                self.assertNotIn("Résultat :", output)

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


if __name__ == "__main__":
    unittest.main(verbosity=2)
