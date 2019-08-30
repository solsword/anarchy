using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Tilemaps;

public class TilePlacer : MonoBehaviour
{
  public Vector2Int where;
  public Vector2Int size;

  public Tile common;
  public Tile uncommon;
  public Tile rare;

  public ulong seed;

  // Start is called before the first frame update
  void Start()
  {
    Anarchy anarchy = GetComponent<Anarchy>();
    for (int x = 0; x < size.x; x += 1) {
      for (int y = 0; y < size.y; y += 1) {
        int i = x*size.x + y;
        ulong s = anarchy.cohort_shuffle(
          (ulong) i,
          (ulong) (size.x*size.y),
          seed
        );

        int xx = where.x + (int) s % size.x;
        int yy = where.y + (int) s / size.x;

        Tile which = common;
        if (i == 0) {
          which = rare;
        } else if (i < 6) {
          which = uncommon;
        }

        Tilemap tilemap = gameObject.GetComponent<Tilemap>();
        if (tilemap != null) {
          tilemap.SetTile(new Vector3Int(xx, yy, 0), which);
        }
      }
    }
  }

  // Update is called once per frame
  void Update()
  {
      
  }
}
