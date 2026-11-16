"""Validate the minimal SQLite schema against GameSvr's actual startup queries."""
from pathlib import Path
import re
import sqlite3
import unittest


SERVER = Path(__file__).resolve().parents[1]


class MinimalSchemaTests(unittest.TestCase):
    def test_startup_queries(self):
        schema = (SERVER / "SQL_Scripts/sqlite_minimal.sql").read_text(encoding="utf-8-sig")
        source = (SERVER / "GameSvr/Handler_CommonDB.cpp").read_text(encoding="gbk")
        queries = re.findall(r'"(SELECT [^"\r\n]+)"', source)
        self.assertGreaterEqual(len(queries), 17)
        with sqlite3.connect(":memory:") as db:
            db.executescript(schema)
            db.executescript(schema)
            for query in queries:
                # The startup source formats only the numeric server index.
                query = query.replace("%d", "1")
                with self.subTest(query=query):
                    self.assertNotIn("%", query)
                    db.execute(query).fetchall()
            self.assertEqual(db.execute(
                "SELECT FLD_INDEX, FLD_STDMODE, FLD_DURAMAX FROM TBL_STDITEM ORDER BY FLD_INDEX"
            ).fetchall(), [(7, 5, 4000), (33, 10, 5000), (34, 11, 5000)])
            self.assertEqual(db.execute(
                "SELECT FLD_INDEX, FLD_VAL1 FROM TBL_STDITEM_ETC"
            ).fetchall(), [(0, 8000)])
            # Reapplying the schema must preserve customized definitions.
            db.execute("UPDATE TBL_STDITEM SET FLD_NAME = 'Custom sword' WHERE FLD_INDEX = 7")
            db.executescript(schema)
            self.assertEqual(db.execute(
                "SELECT FLD_NAME FROM TBL_STDITEM WHERE FLD_INDEX = 7"
            ).fetchall(), [('Custom sword',)])
            self.assertEqual(db.execute("PRAGMA integrity_check").fetchone()[0], "ok")
            self.assertEqual(db.execute("SELECT COUNT(*) FROM TBL_MAPINFO").fetchone()[0], 1)


if __name__ == "__main__":
    unittest.main()
