/*
 * Copyright (c) 2012 Mario Freitas (imkira@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

using UnityEngine;

[ExecuteInEditMode]
[AddComponentMenu("EZ GUI/Controls/SysFont Text")]
public class UISysFontText : SpriteRoot, ISysFontTexturable
{
  [SerializeField]
  protected SysFontTexture _texture = new SysFontTexture();

  #region ISysFontTexturable properties
  public string Text
  {
    get
    {
      return _texture.Text;
    }
    set
    {
      _texture.Text = value;
    }
  }

  public string AppleFontName
  {
    get
    {
      return _texture.AppleFontName;
    }
    set
    {
      _texture.AppleFontName = value;
    }
  }

  public string AndroidFontName
  {
    get
    {
      return _texture.AndroidFontName;
    }
    set
    {
      _texture.AndroidFontName = value;
    }
  }

  public string FontName
  {
    get
    {
      return _texture.FontName;
    }
    set
    {
      _texture.FontName = value;
    }
  }

  public int FontSize
  {
    get
    {
      return _texture.FontSize;
    }
    set
    {
      _texture.FontSize = value;
    }
  }

  public bool IsBold
  {
    get
    {
      return _texture.IsBold;
    }
    set
    {
      _texture.IsBold = value;
    }
  }

  public bool IsItalic
  {
    get
    {
      return _texture.IsItalic;
    }
    set
    {
      _texture.IsItalic = value;
    }
  }

  public SysFont.Alignment Alignment
  {
    get
    {
      return _texture.Alignment;
    }
    set
    {
      _texture.Alignment = value;
    }
  }

  public bool IsMultiLine
  {
    get
    {
      return _texture.IsMultiLine;
    }
    set
    {
      _texture.IsMultiLine = value;
    }
  }

  public int MaxWidthPixels
  {
    get
    {
      return _texture.MaxWidthPixels;
    }
    set
    {
      _texture.MaxWidthPixels = value;
    }
  }

  public int MaxHeightPixels
  {
    get
    {
      return _texture.MaxHeightPixels;
    }
    set
    {
      _texture.MaxHeightPixels = value;
    }
  }

  public int WidthPixels
  {
    get
    {
      return _texture.WidthPixels;
    }
  }

  public int HeightPixels
  {
    get
    {
      return _texture.HeightPixels;
    }
  }

  public int TextWidthPixels
  {
    get
    {
      return _texture.TextWidthPixels;
    }
  }

  public int TextHeightPixels
  {
    get
    {
      return _texture.TextHeightPixels;
    }
  }

  public Texture2D Texture
  {
    get
    {
      return _texture.Texture;
    }
  }
  #endregion

  [SerializeField]
  protected Color _fontColor = Color.white;

  [SerializeField]
  protected SysFontText.PivotAlignment _pivot = SysFontText.PivotAlignment.Center;

  protected Color _lastFontColor;
  public Color FontColor
  {
    get
    {
      return _fontColor;
    }
    set
    {
      if (_fontColor != value)
      {
        _fontColor = value;
      }
    }
  }

  protected SysFontText.PivotAlignment _lastPivot;
  public SysFontText.PivotAlignment Pivot
  {
    get
    {
      return _pivot;
    }
    set
    {
      if (_pivot != value)
      {
        _pivot = value;
      }
    }
  }

	static protected Shader _shader = null;
	protected Material _createdMaterial = null;
	protected Material _material = null;
	protected MeshFilter _filter = null;
	protected MeshRenderer _renderer = null;


	#region SpriteRoot

	//! From Sprite.cs
	public override Vector2 GetDefaultPixelSize(PathFromGUIDDelegate guid2Path, AssetLoaderDelegate loader)
	{
		return new Vector2(WidthPixels, HeightPixels);
	}

	//! From Sprite.cs
	public override int GetStateIndex(string stateName)
	{
		return -1;
	}

	//! From Sprite.cs
	public override void SetState(int index)
	{
		// No Implement, No Animation
	}

	//! From Sprite.cs
	protected override void Awake()
	{
		base.Awake();

		Init();
	}

	//! From Sprite.cs
	public override void Start()
	{
		if (m_started)
			return;
		base.Start();

		// 1フレーム無描画状態が見えてしまうので，1度回す
		Update();
	}


	// Resets all sprite values to defaults for reuse:
	public override void Clear()
	{
		base.Clear();
	}

	public void Update()
	{
//		base.Update();

		if (_texture.NeedsRedraw)
		{
			if (_texture.Update() == false)
			{
				return;
			}

			Setup(TextWidthPixels, TextHeightPixels);

			_material.mainTexture = Texture;
			if(!managed)
				((SpriteMesh)m_spriteMesh).material = _material;
		}

		if (_texture.IsUpdated == false)
		{
			return;
		}

		if ((_fontColor != _lastFontColor) && (_material != null))
		{
			_material.color = _fontColor;
			_lastFontColor = _fontColor;
		}

		if (_lastPivot != _pivot)
		{
			UpdatePivot();
		}
	}

	private void Setup(float w, float h)
	{
		width = w;
		height = h;
		uvsInitialized = false;

		UpdateMesh();

		Init();
	}

	protected void UpdateMesh()
	{
		if (_filter == null)
		{
			_filter = gameObject.GetComponent<MeshFilter>();
			if (_filter == null)
			{
				_filter = gameObject.AddComponent<MeshFilter>();
				_filter.hideFlags = HideFlags.HideInInspector;
			}
		}

		if (_renderer == null)
		{
			_renderer = gameObject.GetComponent<MeshRenderer>();
			if (_renderer == null)
			{
				_renderer = gameObject.AddComponent<MeshRenderer>();
				_renderer.hideFlags = HideFlags.HideInInspector;
			}

			if (_shader == null)
			{
				_shader = Shader.Find("SysFont/Unlit Transparent");
			}

			if (_createdMaterial == null)
			{
				_createdMaterial = new Material(_shader);
			}
			_createdMaterial.hideFlags = HideFlags.HideInInspector | HideFlags.DontSave;
			_material = _createdMaterial;
//			_renderer.sharedMaterial = _material;
		}

		_material.color = _fontColor;
		_lastFontColor = _fontColor;

		UpdatePivot();
	}

	protected void UpdatePivot()
	{
		float ox;
		float oy;

		// horizontal
		if ((_pivot == SysFontText.PivotAlignment.TopLeft) ||
			(_pivot == SysFontText.PivotAlignment.Left) ||
			(_pivot == SysFontText.PivotAlignment.BottomLeft))
		{
			ox = width / 2.0f;
		}
		else if ((_pivot == SysFontText.PivotAlignment.TopRight) ||
			(_pivot == SysFontText.PivotAlignment.Right) ||
			(_pivot == SysFontText.PivotAlignment.BottomRight))
		{
			ox = - width / 2.0f;
		}
		else
		{
			ox = 0.0f;
		}

		// vertical
		if ((_pivot == SysFontText.PivotAlignment.TopLeft) ||
			(_pivot == SysFontText.PivotAlignment.Top) ||
			(_pivot == SysFontText.PivotAlignment.TopRight))
		{
			oy = - height / 2.0f;
		}
		else if ((_pivot == SysFontText.PivotAlignment.BottomLeft) ||
			(_pivot == SysFontText.PivotAlignment.Bottom) ||
			(_pivot == SysFontText.PivotAlignment.BottomRight))
		{
			oy = height / 2.0f;
		}
		else
		{
			oy = 0.0f;
		}

		SetOffset(new Vector3(ox, oy, 0.0f));

		_lastPivot = _pivot;
	}

	/// <summary>
	/// Copies all the attributes of another sprite.
	/// </summary>
	/// <param name="s">A reference to the sprite to be copied.</param>
	public override void Copy(SpriteRoot s)
	{
		UISysFontText sp;

		base.Copy(s);

		// Check the type:
		if (!(s is UISysFontText))
			return;

		sp = (UISysFontText)s;

		sp.Text = Text;
		sp.AppleFontName = AppleFontName;
		sp.AndroidFontName = AndroidFontName;
		sp.FontName = FontName;
		sp.FontSize = FontSize;
		sp.IsBold = IsBold;
		sp.IsItalic = IsItalic;
		sp.Alignment = Alignment;
		sp.IsMultiLine = IsMultiLine;
		sp._texture.MaxWidthPixels = MaxWidthPixels;
		sp._texture.MaxHeightPixels = MaxHeightPixels;
		sp.FontColor = FontColor;
		sp.Pivot = Pivot;
/*
		sp._texture.WidthPixels = WidthPixels;
		sp._texture.HeightPixels = HeightPixels;
		sp._texture.TextWidthPixels = TextWidthPixels;
		sp._texture.TextHeightPixels = TextHeightPixels;
*/

		InitUVs();

		if (autoResize || pixelPerfect)
			CalcSize();
		else
			SetSize(s.width, s.height);
	}


	// Implements UV calculation
	public override void InitUVs()
	{
		uvRect.x = 0.0f;
		uvRect.y = 0.0f;
		uvRect.xMax = _texture.TextWidthPixels / (float)_texture.WidthPixels;
		uvRect.yMax = _texture.TextHeightPixels / (float)_texture.HeightPixels;

/*
		frameInfo.uvs = uvRect;
*/
		SetUVs(uvRect);
	}

	// Ensures that the sprite is updated in the scene view
	// while editing:
	public override void DoMirror()
	{
		// Only run if we're not playing:
		if (Application.isPlaying)
			return;

		// This means Awake() was recently called, meaning
		// we couldn't reliably get valid camera viewport
		// sizes, so we zeroed them out so we'd know to
		// get good values later on (when OnDrawGizmos()
		// is called):
		if (screenSize.x == 0 || screenSize.y == 0)
			base.Start();

		if (mirror == null)
		{
			mirror = new UISysFontTextMirror();
			mirror.Mirror(this);
		}

		mirror.Validate(this);

		// Compare our mirrored settings to the current settings
		// to see if something was changed:
		if (mirror.DidChange(this))
		{
			Init();
			mirror.Mirror(this);	// Update the mirror
		}
	}

	/// <summary>
	/// Creates a GameObject and attaches this
	/// component type to it.
	/// </summary>
	/// <param name="name">Name to give to the new GameObject.</param>
	/// <param name="pos">Position, in world space, where the new object should be created.</param>
	/// <returns>Returns a reference to the component.</returns>
	static public UISysFontText Create(string name, Vector3 pos)
	{
		GameObject go = new GameObject(name);
		go.transform.position = pos;
		return (UISysFontText)go.AddComponent(typeof(UISysFontText));
	}

	/// <summary>
	/// Creates a GameObject and attaches this
	/// component type to it.
	/// </summary>
	/// <param name="name">Name to give to the new GameObject.</param>
	/// <param name="pos">Position, in world space, where the new object should be created.</param>
	/// <param name="rotation">Rotation of the object.</param>
	/// <returns>Returns a reference to the component.</returns>
	static public UISysFontText Create(string name, Vector3 pos, Quaternion rotation)
	{
		GameObject go = new GameObject(name);
		go.transform.position = pos;
		go.transform.rotation = rotation;
		return (UISysFontText)go.AddComponent(typeof(UISysFontText));
	}
	#endregion

	public override void Delete()
	{
		if (_texture != null)
		{
			_texture.Destroy();
			_texture = null;
		}
		SysFont.SafeDestroy(_createdMaterial);
		_createdMaterial = null;
		_material = null;
		_filter = null;
		_renderer = null;

		base.Delete();
	}
	#region MonoBehaviour methods
	#endregion
}

// Mirrors the editable settings of a sprite that affect
// how the sprite is drawn in the scene view
public class UISysFontTextMirror : SpriteRootMirror
{
	public string Text
	{
		get;
		set;
	}

	public string AppleFontName
	{
		get;
		set;
	}

	public string AndroidFontName
	{
		get;
		set;
	}

	public string FontName
	{
		get;
		set;
	}

	public int FontSize
	{
		get;
		set;
	}

	public bool IsBold
	{
		get;
		set;
	}

	public bool IsItalic
	{
		get;
		set;
	}

	public SysFont.Alignment Alignment
	{
		get;
		set;
	}

	public bool IsMultiLine
	{
		get;
		set;
	}

	public int MaxWidthPixels
	{
		get;
		set;
	}

	public int MaxHeightPixels
	{
		get;
		set;
	}

	public int WidthPixels
	{
		get;
		set;
	}

	public int HeightPixels
	{
		get;
		set;
	}

	public int TextWidthPixels
	{
		get;
		set;
	}

	public int TextHeightPixels
	{
		get;
		set;
	}

	public Color FontColor
	{
		get;
		set;
	}

	public SysFontText.PivotAlignment Pivot
	{
		get;
		set;
	}

	// Mirrors the specified sprite's settings
	public override void Mirror(SpriteRoot s)
	{
		base.Mirror(s);

//		lowerLeftPixel = ((Sprite)s).lowerLeftPixel;
//		pixelDimensions = ((Sprite)s).pixelDimensions;
		UISysFontText sp = (UISysFontText)s;

		Text = sp.Text;
		AppleFontName = sp.AppleFontName;
		AndroidFontName = sp.AndroidFontName;
		FontName = sp.FontName;
		FontSize = sp.FontSize;
		IsBold = sp.IsBold;
		IsItalic = sp.IsItalic;
		Alignment = sp.Alignment;
		IsMultiLine = sp.IsMultiLine;
		MaxWidthPixels = sp.MaxWidthPixels;
		MaxHeightPixels = sp.MaxHeightPixels;
		WidthPixels = sp.WidthPixels;
		HeightPixels = sp.HeightPixels;
		TextWidthPixels = sp.TextWidthPixels;
		TextHeightPixels = sp.TextHeightPixels;
		FontColor = sp.FontColor;
		Pivot = sp.Pivot;
	}

	// Returns true if any of the settings do not match:
	public override bool DidChange(SpriteRoot s)
	{
		if (base.DidChange(s))
			return true;

		UISysFontText sp = (UISysFontText)s;

		if(
			(Text != sp.Text) ||
			(AppleFontName != sp.AppleFontName) ||
			(AndroidFontName != sp.AndroidFontName) ||
			(FontName != sp.FontName) ||
			(FontSize != sp.FontSize) ||
			(IsBold != sp.IsBold) ||
			(IsItalic != sp.IsItalic) ||
			(Alignment != sp.Alignment) ||
			(IsMultiLine != sp.IsMultiLine) ||
			(MaxWidthPixels != sp.MaxWidthPixels) ||
			(MaxHeightPixels != sp.MaxHeightPixels) ||
			(WidthPixels != sp.WidthPixels) ||
			(HeightPixels != sp.HeightPixels) ||
			(TextWidthPixels != sp.TextWidthPixels) ||
			(TextHeightPixels != sp.TextHeightPixels) ||
			(FontColor != sp.FontColor) ||
			(Pivot != sp.Pivot)
		)
		{
			s.uvsInitialized = false;
			return true;
		}

		return false;
	}
}